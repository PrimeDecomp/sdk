#include <SDKVersion.h>
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
#include "dolphin/vi.h"
#include "dolphin/OSRtcPriv.h"
#else
#include "dolphin/gx.h"
#endif
#include "dolphin/hw_regs.h"
#include "dolphin/os.h"
#include "dolphin/OSRtcPriv.h"
#include "dolphin/vi.h"

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
// Useful macros.
#define CLAMP(x, l, h) (((x) > (h)) ? (h) : (((x) < (l)) ? (l) : (x)))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define IS_LOWER_16MB(x) ((x) < 16 * 1024 * 1024)
#define ToPhysical(fb) (u32)(((u32)(fb)) & 0x3FFFFFFF)
#define ONES(x) ((1 << (x)) - 1)
#define VI_BITMASK(index) (1ull << (63 - (index)))

const char* __VIVersion = "<< Dolphin SDK - VI\trelease build: Sep  5 2002 05:33:13 (0x2301) >>";

static BOOL IsInitialized;
static vu32 retraceCount;
static u32 flushFlag;
static OSThreadQueue retraceQueue;
static VIRetraceCallback PreCB;
static VIRetraceCallback PostCB;
static u32 encoderType;

static s16 displayOffsetH;
static s16 displayOffsetV;

static vu32 changeMode;
static vu64 changed;

static vu32 shdwChangeMode;
static vu64 shdwChanged;

static VITimingInfo* CurrTiming;
static u32 CurrTvMode;

static u32 NextBufAddr;
static u32 CurrBufAddr;
#else
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
#include "dolphin/dvd.h"

extern volatile u32 Vdac_Flag_Changed;

s8 SCGetDisplayOffsetH(void);
u8 SCGetScreenSaverMode(void);
BOOL __OSSetVIForceDimming(BOOL enable, u32 a, u32 b);
void __DVDRestartMotor(void);
void __DVDStopMotorAsync(DVDCommandBlock* block, DVDCBCallback callback);
extern DVDCommandBlock __DVDStopMotorCommandBlock;
#endif

void __VIInit(VITVMode mode);
void __VISetAdjustingValues(s16 x, s16 y);
void __VIGetAdjustingValues(s16* x, s16* y);
void __VIGetCurrentPosition(s16* x, s16* y);

#define SET_REG_FIELD(line, reg, size, shift, val)                                               \
  do {                                                                                           \
    (reg) = ((u32)__rlwimi((u32)(reg), (val), (shift), 32 - (shift) - (size), 31 - (shift)));    \
  } while (0)

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
const char* __VIVersion = "<< RVL_SDK - VI 	release build: Aug  8 2007 02:07:17 (0x4199_60831) >>";
#else
const char* __VIVersion = "<< Dolphin SDK - VI	release build: Apr  7 2004 04:13:59 (0x2301) >>";
#endif

typedef struct {
    u8 equ;
    u16 acv;
    u16 prbOdd;
    u16 prbEven;
    u16 psbOdd;
    u16 psbEven;
    u8 bs1;
    u8 bs2;
    u8 bs3;
    u8 bs4;
    u16 be1;
    u16 be2;
    u16 be3;
    u16 be4;
    u16 nhlines;
    u16 hlw;
    u8 hsy;
    u8 hcs;
    u8 hce;
    u8 hbe640;
    u16 hbs640;
    u8 hbeCCIR656;
    u16 hbsCCIR656;
} VITiming;

typedef struct {
    u16 DispPosX;
    u16 DispPosY;
    u16 DispSizeX;
    u16 DispSizeY;
    u16 AdjustedDispPosX;
    u16 AdjustedDispPosY;
    u16 AdjustedDispSizeY;
    u16 AdjustedPanPosY;
    u16 AdjustedPanSizeY;
    u16 FBSizeX;
    u16 FBSizeY;
    u16 PanPosX;
    u16 PanPosY;
    u16 PanSizeX;
    u16 PanSizeY;
    VIXFBMode FBMode;
    u32 nonInter;
    u32 tv;
    u8 wordPerLine;
    u8 std;
    u8 wpl;
    u32 bufAddr;
    u32 tfbb;
    u32 bfbb;
    u8 xof;
    BOOL black;
    BOOL threeD;
    u32 rbufAddr;
    u32 rtfbb;
    u32 rbfbb;
    VITiming* timing;
} SomeVIStruct;

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
static BOOL IsInitialized = FALSE;
static volatile u32 retraceCount;
static volatile u32 flushFlag;
static volatile u32 flushFlag3in1;
static vu32 vsync_timing_err_cnt = 0;
static vu32 vsync_timing_test_flag = 0;

static volatile BOOL __VIDimming_All_Clear = FALSE;
static volatile BOOL __VIDimmingFlag_Enable;
static volatile BOOL __VIDVDStopFlag_Enable;
static volatile VITimeToDIM g_current_time_to_dim;
static vu32 THD_TIME_TO_DIMMING = 0;
static vu32 NEW_TIME_TO_DIMMING = 0;
static vu32 THD_TIME_TO_DVD_STOP = 0;
static vu32 _gIdleCount_dimming = 0;
static vu32 _gIdleCount_dvd = 0;
static vu32 __VIDimmingFlag_RF_IDLE;
static vu32 __VIDimmingFlag_SI_IDLE;
static vu32 __VIDimmingFlag_DEV_IDLE[10];
static volatile BOOL __VIDimmingState = FALSE;

static OSThreadQueue retraceQueue;
static void (*PreCB)(u32);
static void (*PostCB)(u32);
static void (*PositionCallback)(s16, s16) = NULL;
static u32 encoderType;
static s16 displayOffsetH = 0;
static s16 displayOffsetV = 0;
static vu32 changeMode = 0;
static vu64 changed = 0;
static u16 regs[59];
static vu32 shdwChangeMode = 0;
static vu64 shdwChanged = 0;
static u16 shdwRegs[59];
static SomeVIStruct HorVer;
static VITiming* CurrTiming;
static u32 CurrTvMode;
static u32 NextBufAddr;
static u32 CurrBufAddr;
static u32 FBSet = 0;
#else
static BOOL IsInitialized;
static volatile u32 retraceCount;

static volatile u32 flushFlag;
static OSThreadQueue retraceQueue;
static void (*PreCB)(u32);
static void (*PostCB)(u32);
static void (*PositionCallback)(s16, s16);
static u32 encoderType;
static s16 displayOffsetH;
static s16 displayOffsetV;
static volatile u32 changeMode;
static volatile u64 changed;
static volatile u32 shdwChangeMode;
static u16 regs[59];
static volatile u64 shdwChanged;
static VITiming* CurrTiming;
static u32 CurrTvMode;
static u32 NextBufAddr;
static u32 CurrBufAddr;
static u16 shdwRegs[59];
#endif
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
static u32 FBSet;
#else
#define MARK_CHANGED(index) (changed |= 1LL << (63 - (index)))

static VITiming timing[] = {
    { 6, 240, 24, 25, 3, 2, 12, 13, 12, 13, 520, 519, 520, 519, 525, 429, 64, 71, 105, 162, 373, 122, 412 },
    { 6, 240, 24, 24, 4, 4, 12, 12, 12, 12, 520, 520, 520, 520, 526, 429, 64, 71, 105, 162, 373, 122, 412 },
    { 5, 287, 35, 36, 1, 0, 13, 12, 11, 10, 619, 618, 617, 620, 625, 432, 64, 75, 106, 172, 380, 133, 420 },
    { 5, 287, 33, 33, 2, 2, 13, 11, 13, 11, 619, 621, 619, 621, 624, 432, 64, 75, 106, 172, 380, 133, 420 },
    { 6, 240, 24, 25, 3, 2, 16, 15, 14, 13, 518, 517, 516, 519, 525, 429, 64, 78, 112, 162, 373, 122, 412 },
    { 6, 240, 24, 24, 4, 4, 16, 14, 16, 14, 518, 520, 518, 520, 526, 429, 64, 78, 112, 162, 373, 122, 412 },
    { 12, 480, 48, 48, 6, 6, 24, 24, 24, 24, 1038, 1038, 1038, 1038, 1050, 429, 64, 71, 105, 162, 373, 122, 412 },
    { 12, 480, 44, 44, 10, 10, 24, 24, 24, 24, 1038, 1038, 1038, 1038, 1050, 429, 64, 71, 105, 168, 379, 122, 412 },
    { 6, 241, 24, 25, 1, 0, 12, 13, 12, 13, 520, 519, 520, 519, 525, 429, 64, 71, 105, 159, 370, 122, 412 },
    { 12, 480, 48, 48, 6, 6, 24, 24, 24, 24, 1038, 1038, 1038, 1038, 1050, 429, 64, 71, 105, 180, 391, 122, 412 },
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    { 10, 576, 62, 62, 6, 6, 20, 20, 20, 20, 1240, 1240, 1240, 1240, 1250, 432, 64, 75, 106, 172, 380, 122, 412 }
#endif
};

static u16 taps[25] = {
    0x01F0, 0x01DC,
    0x01AE, 0x0174,
    0x0129, 0x00DB,
    0x008E, 0x0046,
    0x000C, 0x00E2,
    0x00CB, 0x00C0,
    0x00C4, 0x00CF,
    0x00DE, 0x00EC,
    0x00FC, 0x0008,
    0x000F, 0x0013,
    0x0013, 0x000F,
    0x000C, 0x0008,
    0x0001
};

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
static VITiming* timingExtra = NULL;
#else
static SomeVIStruct HorVer;
static u32 FBSet;
static VITiming* timingExtra;
#endif
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
static vu16 regs[59];
static vu16 shdwRegs[59];

static VIPositionInfo HorVer;
// clang-format off
static VITimingInfo timing[10] = { 
	{ // NTSC INT
		6, 240, 24, 25, 3, 2, 12, 13, 12, 13, 520, 519, 520, 519, 525, 429, 64, 71, 105, 162, 373, 122, 412,
	},
	{ // NTSC DS
		6, 240, 24, 24, 4, 4, 12, 12, 12, 12, 520, 520, 520, 520, 526, 429, 64, 71, 105, 162, 373, 122, 412,
	},
	{ // PAL INT
		5, 287, 35, 36, 1, 0, 13, 12, 11, 10, 619, 618, 617, 620, 625, 432, 64, 75, 106, 172, 380, 133, 420,
	}, 
	{ // PAL DS
		5, 287, 33, 33, 2, 2, 13, 11, 13, 11, 619, 621, 619, 621, 624, 432, 64, 75, 106, 172, 380, 133, 420,
	}, 	
	{ // MPAL INT
		6, 240, 24, 25, 3, 2, 16, 15, 14, 13, 518, 517, 516, 519, 525, 429, 64, 78, 112, 162, 373, 122, 412,
	},
	{ // MPAL DS
		6, 240, 24, 24, 4, 4, 16, 14, 16, 14, 518, 520, 518, 520, 526, 429, 64, 78, 112, 162, 373, 122, 412,
	},
	{ // NTSC PRO
		12, 480, 48, 48, 6, 6, 24, 24, 24, 24, 1038, 1038, 1038, 1038, 1050, 429, 64, 71, 105, 162, 373, 122, 412,
	},
	{ // NTSC 3D
		12, 480, 44, 44, 10, 10, 24, 24, 24, 24, 1038, 1038, 1038, 1038, 1050, 429, 64, 71, 105, 168, 379, 122, 412,
	},
	{ // GCA INT
		6, 241, 24, 25, 1, 0, 12, 13, 12, 13, 520, 519, 520, 519, 525, 429, 64, 71, 105, 159, 370, 122, 412,
	},
	{ // GCA DS
		12, 480, 48, 48, 6, 6, 24, 24, 24, 24, 1038, 1038, 1038, 1038, 1050, 429, 64, 71, 105, 180, 391, 122, 412,
	},
};
// clang-format on

static u16 taps[26] = {496, 476, 430, 372, 297, 219, 142, 70, 12, 226, 203, 192, 196,
                       207, 222, 236, 252, 8,   15,  19,  19, 15, 12,  8,   1,   0};

// forward declaring statics
static u32 getCurrentFieldEvenOdd();

static void getEncoderType(void) {
  // UNUSED FUNCTION
#else
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
GXRenderModeObj GXPal528Prog = {
    VI_TVMODE_PAL_PROG,
    640,
    528,
    528,
    (720 - 640) / 2,
    (574 - 528) / 2,
    640,
    528,
    VI_XFBMODE_SF,
    GX_FALSE,
    GX_FALSE,
    { { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },
    { 0, 0, 21, 22, 21, 0, 0 },
};

GXRenderModeObj GXPal528ProgSoft = {
    VI_TVMODE_PAL_PROG,
    640,
    528,
    528,
    (720 - 640) / 2,
    (574 - 528) / 2,
    640,
    528,
    VI_XFBMODE_SF,
    GX_FALSE,
    GX_FALSE,
    { { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },
    { 8, 8, 10, 12, 10, 8, 8 },
};

GXRenderModeObj GXPal524ProgAa = {
    VI_TVMODE_PAL_PROG,
    640,
    264,
    524,
    (720 - 640) / 2,
    (574 - 528) / 2,
    640,
    524,
    VI_XFBMODE_SF,
    GX_FALSE,
    GX_TRUE,
    { { 3, 2 }, { 9, 6 }, { 3, 10 }, { 3, 2 }, { 9, 6 }, { 3, 10 }, { 9, 2 }, { 3, 6 }, { 9, 10 }, { 9, 2 }, { 3, 6 }, { 9, 10 } },
    { 4, 8, 12, 16, 12, 8, 4 },
};

static BOOL OnShutdown(BOOL final, u32 event);
static OSShutdownFunctionInfo ShutdownFunctionInfo = { OnShutdown, 127 };

static BOOL OnShutdown(BOOL final, u32 event) {
    BOOL retval;
    static BOOL first = TRUE;
    static u32 count;

    if (final == FALSE) {
        switch (event) {
        case 3:
        case 1:
        case 2:
            if (first) {
                VISetRGBModeImm();
                VIFlush();
                count = retraceCount;
                first = FALSE;
                retval = FALSE;
            } else {
                if (count == retraceCount) {
                    retval = FALSE;
                } else {
                    retval = TRUE;
                }
            }
            break;

        case 4:
        case 0:
        case 6:
        case 5:
            retval = TRUE;
            break;
        }
    } else {
        retval = TRUE;
    }

    return retval;
}
#endif

// prototypes
static u32 getCurrentFieldEvenOdd(void);
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
static void GetCurrentDisplayPosition(u32* hct, u32* vct);
#endif
VITiming* __VISetExtraTiming(VITiming* t);
void __VIEnableRawPositionInterrupt(s16 x, s16 y, void (*callback)(s16, s16));
void (*__VIDisableRawPositionInterrupt())(s16, s16);
void __VIDisplayPositionToXY(u32 hct, u32 vct, s16* x, s16* y);
void __VISetLatchMode(u32 mode);
int __VIGetLatch0Position(s16* px, s16* py);
int __VIGetLatch1Position(s16* px, s16* py);
int __VIGetLatchPosition(u32 port, s16* px, s16* py);


#if SDK_VERSION < SDK_VERSION_RVL_2008_01
static u32 getEncoderType(void) {
    return 1;
}
#endif

static s32 cntlzd(u64 bit) {
    u32 hi;
    u32 lo;
    s32 value;

    hi = bit >> 32;
    lo = bit & 0xFFFFFFFF;
    value = __cntlzw(hi);
    if (value < 32) {
        return value;
    }
    return __cntlzw(lo) + 32;
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
static int cntlzd(u64 bit) {
  u32 hi, lo;
  int value;
#else
static int VISetRegs(void) {
    s32 regIndex;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  hi = (u32)(bit >> 32);
  lo = (u32)(bit & 0xFFFFFFFF);
  value = __cntlzw(hi);
#else
    if (shdwChangeMode != 1 || getCurrentFieldEvenOdd() != 0) {
        while (shdwChanged != 0) {
            regIndex = cntlzd(shdwChanged);
            __VIRegs[regIndex] = shdwRegs[regIndex];
            shdwChanged &= ~((u64)1 << (63 - regIndex));
        }
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  if (value < 32) {
    return value;
  }
#else
        shdwChangeMode = 0;
        CurrTiming = HorVer.timing;
        CurrTvMode = HorVer.tv;
        CurrBufAddr = NextBufAddr;
        return 1;
    }
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  return (32 + __cntlzw(lo));
#else
    return 0;
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
static BOOL VISetRegs(void) {
  int regIndex;
#else
static void __VIRetraceHandler(__OSInterrupt unused, OSContext* context) {
    OSContext exceptionContext;
    u16 reg;
    u32 inter;
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    u32 regIndex;
    static u32 old_dtvStatus = 999;
    static u32 old_tvtype = 999;
    u32 now_dtvStatus = 0;
    u32 now_tvtype = 0;
    static BOOL __VIDimmingFlag_Enable_old = TRUE;
    static BOOL __VIDVDStopFlag_Enable_old = TRUE;
    u32 i;
    static u32 DimmingON_Pending = 0;
    static u32 DimmingOFF_Pending = 0;
#endif
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  if (!((shdwChangeMode == 1) && (getCurrentFieldEvenOdd() == 0))) {
    while (shdwChanged) {
      regIndex = cntlzd(shdwChanged);
      __VIRegs[regIndex] = shdwRegs[regIndex];
      shdwChanged &= ~(VI_BITMASK(regIndex));
#else
    inter = 0;
    reg = __VIRegs[0x18];
    if (reg & 0x8000) {
        __VIRegs[0x18] = reg & ~0x8000;
        inter |= 1;
    }
    reg = __VIRegs[0x1A];
    if (reg & 0x8000) {
        __VIRegs[0x1A] = reg & ~0x8000;
        inter |= 2;
    }
    reg = __VIRegs[0x1C];
    if (reg & 0x8000) {
        __VIRegs[0x1C] = reg & ~0x8000;
        inter |= 4;
    }
    reg = __VIRegs[0x1E];
    if (reg & 0x8000) {
        __VIRegs[0x1E] = reg & ~0x8000;
        inter |= 8;
    }
    reg = __VIRegs[0x1E];

    if ((inter & 4) || (inter & 8)) {
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);

        if (PositionCallback != 0) {
            s16 x, y;
            __VIGetCurrentPosition(&x, &y);
            (*PositionCallback)(x, y);
        }

        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
        return;
#endif
    }

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
#if SDK_VERSION < SDK_VERSION_RVL_2008_01
    if (inter == 0) {
    }
#endif

    retraceCount += 1;
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);

    if (PreCB) {
        PreCB(retraceCount);
    }

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    if (vsync_timing_test_flag) {
        u32 hcount, vcount;

        GetCurrentDisplayPosition(&hcount, &vcount);

        if (!(vcount == 1 || vcount == CurrTiming->nhlines / 2 + 1)) {
            vsync_timing_err_cnt++;
        }
    }
#endif

    if (flushFlag != 0) {
        if (VISetRegs() != 0) {
            flushFlag = 0;
            SIRefreshSamplingRate();
        }
    }

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    now_dtvStatus = VIGetDTVStatus();
    if (now_dtvStatus != old_dtvStatus) {
        __VISetYUVSEL((VIBool)now_dtvStatus);
    }

    old_dtvStatus = now_dtvStatus;
    now_tvtype = VIGetTvFormat();

    if (now_tvtype != old_tvtype) {
        if (now_tvtype == VI_EURGB60) {
            __VISetFilter4EURGB60(VI_ENABLE);
        } else {
            __VISetFilter4EURGB60(VI_DISABLE);
        }

        switch (now_tvtype) {
        case VI_PAL:
            switch (g_current_time_to_dim) {
            case VI_DM_10M:
                NEW_TIME_TO_DIMMING = 30000;
                break;
            case VI_DM_15M:
                NEW_TIME_TO_DIMMING = 45000;
                break;
            default:
                NEW_TIME_TO_DIMMING = 15000;
                break;
            }
            THD_TIME_TO_DVD_STOP = 90000;
            break;
        default:
            switch (g_current_time_to_dim) {
            case VI_DM_10M:
                NEW_TIME_TO_DIMMING = 36000;
                break;
            case VI_DM_15M:
                NEW_TIME_TO_DIMMING = 54000;
                break;
            default:
                NEW_TIME_TO_DIMMING = 18000;
                break;
            }
            THD_TIME_TO_DVD_STOP = 108000;
            break;
        }

        _gIdleCount_dimming = 0;
        _gIdleCount_dvd = 0;
    }

    old_tvtype = now_tvtype;

    if (flushFlag3in1) {
        while (Vdac_Flag_Changed) {
            regIndex = __cntlzw(Vdac_Flag_Changed);
            regIndex = 1 << (31 - regIndex);

            switch (regIndex) {
            case 1:
                __VISetCGMS();
                break;
            case 2:
                __VISetWSS();
                break;
            case 4:
                __VISetClosedCaption();
                break;
            case 8:
                __VISetMacrovision();
                break;
            case 0x10:
                __VISetGamma();
                break;
            case 0x20:
                __VISetTrapFilter();
                break;
            case 0x40:
                __VISetRGBOverDrive();
                break;
            case 0x80:
                __VISetRGBModeImm();
                break;
            }

            Vdac_Flag_Changed &= ~regIndex;
        }

        flushFlag3in1 = 0;
    }
#endif

    if (PostCB) {
        OSClearContext(&exceptionContext);
        PostCB(retraceCount);
    }

    OSWakeupThread(&retraceQueue);
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    if (__VIDimming_All_Clear == TRUE) {
        if (__OSSetVIForceDimming(FALSE, 0, 0) == TRUE) {
            __VIDimming_All_Clear = FALSE;
            _gIdleCount_dimming = 0;
        }
    }

    for (i = 0; i < 10; i++) {
        if (__VIDimmingFlag_DEV_IDLE[i] == 0) {
            __VIDimmingFlag_DEV_IDLE[0] = 0;
            break;
        }
    }

    if (__VIDimmingFlag_RF_IDLE && __VIDimmingFlag_SI_IDLE && __VIDimmingFlag_DEV_IDLE[0]) {
        if (__VIDimmingFlag_Enable == TRUE && _gIdleCount_dimming < 0xFFFFFFFF) {
            _gIdleCount_dimming++;
        }

        if (__VIDVDStopFlag_Enable == TRUE && _gIdleCount_dvd < 0xFFFFFFFF) {
            _gIdleCount_dvd++;
        }
    } else {
        if (_gIdleCount_dimming >= THD_TIME_TO_DIMMING) {
            DimmingOFF_Pending = 1;
        }
        if (_gIdleCount_dvd >= THD_TIME_TO_DVD_STOP) {
            __DVDRestartMotor();
        }

        _gIdleCount_dimming = 0;
        _gIdleCount_dvd = 0;
        THD_TIME_TO_DIMMING = NEW_TIME_TO_DIMMING;
    }

    if (__VIDimmingFlag_Enable_old != __VIDimmingFlag_Enable) {
        if (__VIDimmingFlag_Enable == FALSE) {
            if (_gIdleCount_dimming >= THD_TIME_TO_DIMMING) {
                DimmingOFF_Pending = 1;
            }
        }

        _gIdleCount_dimming = 0;
        THD_TIME_TO_DIMMING = NEW_TIME_TO_DIMMING;
    }

    if (_gIdleCount_dimming == THD_TIME_TO_DIMMING) {
        DimmingON_Pending = 1;
    }

    if (DimmingOFF_Pending) {
        if (__OSSetVIForceDimming(FALSE, 2, 2) == TRUE) {
            DimmingOFF_Pending = 0;
            __VIDimmingState = FALSE;
        }
    }

    if (DimmingON_Pending) {
        if (__OSSetVIForceDimming(TRUE, 2, 2) == TRUE) {
            DimmingON_Pending = 0;
            __VIDimmingState = TRUE;
        }
    }

    if (__VIDVDStopFlag_Enable_old != __VIDVDStopFlag_Enable) {
        if (__VIDVDStopFlag_Enable == FALSE) {
            if (_gIdleCount_dvd >= THD_TIME_TO_DVD_STOP) {
                __DVDRestartMotor();
            }
        }
        _gIdleCount_dvd = 0;
    }

    if (_gIdleCount_dvd == THD_TIME_TO_DVD_STOP) {
        __DVDStopMotorAsync(&__DVDStopMotorCommandBlock, NULL);
    }

    __VIDimmingFlag_RF_IDLE = 1;
    __VIDimmingFlag_SI_IDLE = 1;

    for (i = 0; i < 10; i++) {
        __VIDimmingFlag_DEV_IDLE[i] = 1;
    }

    __VIDimmingFlag_Enable_old = __VIDimmingFlag_Enable;
    __VIDVDStopFlag_Enable_old = __VIDVDStopFlag_Enable;

    if (NEW_TIME_TO_DIMMING > _gIdleCount_dimming && __VIDimmingState == FALSE) {
        THD_TIME_TO_DIMMING = NEW_TIME_TO_DIMMING;
    }
#endif
}

VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback cb) {
    BOOL enabled;
    VIRetraceCallback oldcb;

    oldcb = PreCB;
    enabled = OSDisableInterrupts();
    PreCB = cb;
    OSRestoreInterrupts(enabled);
    return oldcb;
}

VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback cb) {
    BOOL enabled;
    VIRetraceCallback oldcb;

    oldcb = PostCB;
    enabled = OSDisableInterrupts();
    PostCB = cb;
    OSRestoreInterrupts(enabled);
    return oldcb;
}

VITiming* __VISetExtraTiming(VITiming* t) {
    VITiming* old = timingExtra;

    timingExtra = t;
    return old;
}

#pragma dont_inline on
static VITiming* getTiming(VITVMode mode) {
    switch (mode) {
    case VI_TVMODE_NTSC_INT:        return &timing[0];
    case VI_TVMODE_NTSC_DS:         return &timing[1];
    case VI_TVMODE_PAL_INT:         return &timing[2];
    case VI_TVMODE_PAL_DS:          return &timing[3];
    case VI_TVMODE_EURGB60_INT:     return &timing[0];
    case VI_TVMODE_EURGB60_DS:      return &timing[1];
    case VI_TVMODE_MPAL_INT:        return &timing[4];
    case VI_TVMODE_MPAL_DS:         return &timing[5];
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    case VI_TVMODE_NTSC_PROG:
    case 10:
    case 22:
        return &timing[6];
#else
    case VI_TVMODE_NTSC_PROG:       return &timing[6];
#endif
    case 3:                         return &timing[7];
    case VI_TVMODE_DEBUG_PAL_INT:   return &timing[2];
    case VI_TVMODE_DEBUG_PAL_DS:    return &timing[3];
    case 24:                        return &timing[8];
    case 26:                        return &timing[9];
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    case 6:                         return &timing[10];
    case 28:
    case 29:
    case 30:
    case 34:
        return timingExtra;
#else
    case 29:
    case 30:
    case 28:
        return timingExtra;
#endif
    default:
        return NULL;
    }
}
#pragma dont_inline reset

void __VIInit(VITVMode mode) {
    VITiming* tm;
    u32 nonInter;
    u32 tv;
    u32 tvForReg;
    volatile u32 a;
    u16 hct;
    u16 vct;
#if SDK_VERSION < SDK_VERSION_RVL_2008_01
    u32 encoderType;

    encoderType = getEncoderType();
    if (encoderType == 0) {
        __VIInitPhilips();
    }
#endif

    nonInter = mode & 3;
    tv = (u32)mode >> 2;
    *(u32*)OSPhysicalToCached(0xCC) = tv;
#if SDK_VERSION < SDK_VERSION_RVL_2008_01
    if (encoderType == 0) {
        tv = 3;
    }
#endif
    tm = getTiming(mode);
    __VIRegs[1] = 2;

    // why?
    for (a = 0; a < 1000; a++) {}

    __VIRegs[1] = 0;
    __VIRegs[3] = (u32)tm->hlw;
    __VIRegs[2] = tm->hce | (tm->hcs << 8);
    __VIRegs[5] = tm->hsy | ((tm->hbe640 & 0x1FF) << 7);
    __VIRegs[4] = (tm->hbe640 >> 9) | ((tm->hbs640 & 0xFFFF) << 1);
    if (encoderType == 0) {
        __VIRegs[0x39] = tm->hbeCCIR656 | 0x8000;
        __VIRegs[0x3A] = (u32)tm->hbsCCIR656;
    }
    __VIRegs[0] = (u32)tm->equ;
    __VIRegs[7] = (u32)(tm->prbOdd + (tm->acv * 2) - 2);
    __VIRegs[6] = (u32)(tm->psbOdd + 2);
    __VIRegs[9] = (u32)(tm->prbEven + (tm->acv * 2) - 2);
    __VIRegs[8] = (u32)(tm->psbEven + 2);
    __VIRegs[11] = tm->bs1 | (tm->be1 << 5);
    __VIRegs[10] = tm->bs3 | (tm->be3 << 5);
    __VIRegs[13] = tm->bs2 | (tm->be2 << 5);
    __VIRegs[12] = tm->bs4 | (tm->be4 << 5);
    __VIRegs[36] = 0x2828;
    __VIRegs[27] = 1;
    __VIRegs[26] = 0x1001;
    hct = tm->hlw + 1;
    vct = (tm->nhlines / 2) + 1;
    __VIRegs[25] = (u16)(u32)hct;
    __VIRegs[24] = vct | 0x1000;

    switch (tv) {
    case 1:
    case 2:
    case 3:
        tvForReg = tv;
        break;
    default:
        tvForReg = 0;
    }

    if (nonInter == 0 || nonInter == 1) {
        __VIRegs[1] = ((nonInter << 2) & 4) | 1 | (tvForReg << 8);
        __VIRegs[54] = 0;
        return;
    }

    __VIRegs[1] = (tvForReg << 8) | 5;
    __VIRegs[54] = 1;
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(val, min, max) ((val) > (max) ? (max) : (val) < (min) ? (min) : (val))

static void AdjustPosition(u16 acv) {
    s32 coeff;
    s32 frac;

    HorVer.AdjustedDispPosX = CLAMP((s16)HorVer.DispPosX + displayOffsetH, 0, 0x2D0 - HorVer.DispSizeX);
    coeff = (HorVer.FBMode == VI_XFBMODE_SF) ? 2 : 1;
    frac = HorVer.DispPosY & 1;
    HorVer.AdjustedDispPosY = MAX((s16)HorVer.DispPosY + displayOffsetV, frac);
    HorVer.AdjustedDispSizeY = HorVer.DispSizeY
                             + MIN((s16)HorVer.DispPosY + displayOffsetV - frac, 0)
                             - MAX((s16)HorVer.DispPosY + (s16)HorVer.DispSizeY + displayOffsetV - (((s16)acv * 2) - frac), 0);
    HorVer.AdjustedPanPosY = HorVer.PanPosY
                           - (MIN((s16)HorVer.DispPosY + displayOffsetV - frac, 0) / coeff);
    HorVer.AdjustedPanSizeY = HorVer.PanSizeY
                            + (MIN((s16)HorVer.DispPosY + displayOffsetV - frac, 0) / coeff)
                            - (MAX((s16)HorVer.DispPosY + (s16)HorVer.DispSizeY + displayOffsetV - (((s16)acv * 2) - frac), 0) / coeff);
}

static void ImportAdjustingValues(void) {
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    displayOffsetH = SCGetDisplayOffsetH();
    displayOffsetV = 0;
#else
    OSSram* sram = __OSLockSram();

    displayOffsetH = sram->displayOffsetH;
    displayOffsetV = 0;
    __OSUnlockSram(0);
#endif
}

void VIInit(void) {
    u16 dspCfg;
    u32 value;
    u32 tv;
    u32 tvInBootrom;

    if (IsInitialized) {
        return;
    }

    OSRegisterVersion(__VIVersion);
    IsInitialized = TRUE;

#if SDK_VERSION < SDK_VERSION_RVL_2008_01
    encoderType = getEncoderType();
#endif
    if (!(__VIRegs[1] & 1)) {
        __VIInit(VI_TVMODE_NTSC_INT);
    }

    retraceCount = 0;
    changed = 0;
    shdwChanged = 0;
    changeMode = 0;
#endif
    shdwChangeMode = 0;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
    flushFlag = 0;
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    flushFlag3in1 = 0;
#endif

    __VIRegs[39] = taps[0] | ((taps[1] & 0x3F) << 10);
    __VIRegs[38] = (taps[1] >> 6) | (taps[2] << 4);
    __VIRegs[41] = taps[3] | ((taps[4] & 0x3F) << 10);
    __VIRegs[40] = (taps[4] >> 6) | (taps[5] << 4);
    __VIRegs[43] = taps[6] | ((taps[7] & 0x3F) << 10);
    __VIRegs[42] = (taps[7] >> 6) | (taps[8] << 4);
    __VIRegs[45] = taps[9] | (taps[10] << 8);
    __VIRegs[44] = taps[11] | (taps[12] << 8);
    __VIRegs[47] = taps[13] | (taps[14] << 8);
    __VIRegs[46] = taps[15] | (taps[16] << 8);
    __VIRegs[49] = taps[17] | (taps[18] << 8);
    __VIRegs[48] = taps[19] | (taps[20] << 8);
    __VIRegs[51] = taps[21] | (taps[22] << 8);
    __VIRegs[50] = taps[23] | (taps[24] << 8);
    __VIRegs[56] = 0x280;
    ImportAdjustingValues();

    tvInBootrom = *(u32*)OSPhysicalToCached(0xCC);
    dspCfg = __VIRegs[1];
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    HorVer.nonInter = VIGetScanMode();
#else
    HorVer.nonInter = (s32) ((dspCfg >> 2U) & 1);
#endif
    HorVer.tv = ((u32)(dspCfg) & 0x300) >> 8;

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    if (tvInBootrom == VI_EURGB60 || (tvInBootrom == VI_PAL && HorVer.tv == VI_NTSC)) {
#else
    if (tvInBootrom == VI_PAL && HorVer.tv == VI_NTSC) {
#endif
        HorVer.tv = VI_EURGB60;
    }

    tv = (HorVer.tv == VI_DEBUG) ? VI_NTSC : HorVer.tv;
    HorVer.timing = getTiming((tv << 2) + HorVer.nonInter);
    regs[1] = dspCfg;

#endif
    CurrTiming = HorVer.timing;
    CurrTvMode = HorVer.tv;
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
    CurrBufAddr = NextBufAddr;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
    return TRUE;
  }
  return FALSE;
#else
    HorVer.DispSizeX = 640;
    HorVer.DispSizeY = CurrTiming->acv * 2;
    HorVer.DispPosX = (720 - HorVer.DispSizeX) / 2;
    HorVer.DispPosY = 0;
    AdjustPosition(CurrTiming->acv);
    HorVer.FBSizeX = 640;
    HorVer.FBSizeY = CurrTiming->acv * 2;
    HorVer.PanPosX = 0;
    HorVer.PanPosY = 0;
    HorVer.PanSizeX = 640;
    HorVer.PanSizeY = CurrTiming->acv * 2;
    HorVer.FBMode = 0;

    HorVer.wordPerLine = 40;
    HorVer.std = 40;
    HorVer.wpl = 40;
    HorVer.xof = 0;
    HorVer.black = 1;
    HorVer.threeD = 0;
    OSInitThreadQueue(&retraceQueue);
    value = __VIRegs[24];
    value &= ~0x8000;
    value = (u16)value;
    __VIRegs[24] = value;
    value = __VIRegs[26];
    value = value & ~0x8000;
    value = (u16)value;
    __VIRegs[26] = value;
    PreCB = NULL;
    PostCB = NULL;
    __OSSetInterruptHandler(0x18, __VIRetraceHandler);
    __OSUnmaskInterrupts(0x80);
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    OSRegisterShutdownFunction(&ShutdownFunctionInfo);

    switch (VIGetTvFormat()) {
    case VI_PAL:
        THD_TIME_TO_DIMMING = 15000;
        NEW_TIME_TO_DIMMING = 15000;
        THD_TIME_TO_DVD_STOP = 90000;
        break;
    default:
        THD_TIME_TO_DIMMING = 18000;
        NEW_TIME_TO_DIMMING = 18000;
        THD_TIME_TO_DVD_STOP = 108000;
        break;
    }
    _gIdleCount_dimming = 0;
    _gIdleCount_dvd = 0;
    g_current_time_to_dim = VI_DM_DEFAULT;
    __VIDimming_All_Clear = TRUE;
    __VIDimmingState = FALSE;
    VIEnableDimming(TRUE);

    VIEnableDVDStopMotor(FALSE);
    __VISetRevolutionModeSimple();
#endif
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
static void __VIRetraceHandler(__OSInterrupt interrupt, OSContext* context) {
  OSContext exceptionContext;
  u16 viReg;
  u32 inter = 0;
#else
void VIWaitForRetrace(void) {
    BOOL enabled;
    u32 count;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  viReg = __VIRegs[VI_DISP_INT_0];
  if (viReg & 0x8000) {
    __VIRegs[VI_DISP_INT_0] = (u16)(viReg & ~0x8000);
    inter |= 1;
  }

  viReg = __VIRegs[VI_DISP_INT_1];
  if (viReg & 0x8000) {
    __VIRegs[VI_DISP_INT_1] = (u16)(viReg & ~0x8000);
    inter |= 2;
  }

  viReg = __VIRegs[VI_DISP_INT_2];
  if (viReg & 0x8000) {
    __VIRegs[VI_DISP_INT_2] = (u16)(viReg & ~0x8000);
    inter |= 4;
  }

  viReg = __VIRegs[VI_DISP_INT_3];
  if (viReg & 0x8000) {
    __VIRegs[VI_DISP_INT_3] = (u16)(viReg & ~0x8000);
    inter |= 8;
  }

  if ((inter & 4) || (inter & 8)) {
    OSSetCurrentContext(context);
    return;
  }

  retraceCount++;

  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
  if (PreCB) {
    (*PreCB)(retraceCount);
  }

  if (flushFlag) {
    if (VISetRegs()) {
      flushFlag = 0;
      SIRefreshSamplingRate();
    }
  }

  if (PostCB) {
    OSClearContext(&exceptionContext);
    (*PostCB)(retraceCount);
  }

  OSWakeupThread(&retraceQueue);
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(context);
#else
    enabled = OSDisableInterrupts();
    count = retraceCount;
    do {
        OSSleepThread(&retraceQueue);
    } while (count == retraceCount);
    OSRestoreInterrupts(enabled);
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	800D0A5C
 * Size:	000044
 */
VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback callback) {
  int interrupt;
  VIRetraceCallback oldCallback;
#else
static void setInterruptRegs(VITiming* tm) {
    u16 hct, vct;
    u16 borrow;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  oldCallback = PreCB;
#else
    vct = tm->nhlines / 2;
    borrow = tm->nhlines % 2;
    if (borrow != 0) {
        hct = tm->hlw;
    } else {
        hct = 0;
    }
    vct++;
    hct++;
    regs[25] = (u16)(u32)hct;
    MARK_CHANGED(25);
    regs[24] = vct | 0x1000;
    MARK_CHANGED(24);
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  interrupt = OSDisableInterrupts();
  PreCB = callback;
  OSRestoreInterrupts(interrupt);

  return oldCallback;
#else
    vct;  // fixes regalloc
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	800D0AA0
 * Size:	000044
 */
VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback callback) {
  int interrupt;
  VIRetraceCallback oldCallback;

  oldCallback = PostCB;

  interrupt = OSDisableInterrupts();
  PostCB = callback;
  OSRestoreInterrupts(interrupt);

  return oldCallback;
#else
static void setPicConfig(u16 fbSizeX, VIXFBMode xfbMode, u16 panPosX, u16 panSizeX, u8* wordPerLine, u8* std, u8* wpl, u8* xof) {
    *wordPerLine = (fbSizeX + 15) / 16;
    *std = (xfbMode == VI_XFBMODE_SF) ? *wordPerLine : (u8)(*wordPerLine * 2);
    *xof = panPosX % 16;
    *wpl = (*xof + panSizeX + 15) / 16;
    regs[0x24] = *std | (*wpl << 8);
    changed |= 0x8000000;
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	800D0AE4
 * Size:	0000A0
 */
static VITimingInfo* getTiming(VITVMode mode) {
  switch (mode) {
  case VI_TVMODE_NTSC_INT:
    return &timing[0];
  case VI_TVMODE_NTSC_DS:
    return &timing[1];
#else
static void setBBIntervalRegs(VITiming* tm) {
    u16 val;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  case VI_TVMODE_PAL_INT:
    return &timing[2];
  case VI_TVMODE_PAL_DS:
    return &timing[3];
#else
    val = tm->bs1 | (tm->be1 << 5);
    regs[11] = val;
    changed |= 0x10000000000000;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  case VI_TVMODE_EURGB60_INT:
    return &timing[0];
  case VI_TVMODE_EURGB60_DS:
    return &timing[1];
#else
    val = tm->bs3 | (tm->be3 << 5);
    regs[10] = val;
    changed |= 0x20000000000000;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  case VI_TVMODE_MPAL_INT:
    return &timing[4];
  case VI_TVMODE_MPAL_DS:
    return &timing[5];
#else
    val = tm->bs2 | (tm->be2 << 5);
    regs[13] = val;
    changed |= 0x4000000000000;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  case VI_TVMODE_NTSC_PROG:
    return &timing[6];
  case VI_TVMODE_NTSC_3D:
    return &timing[7];

  case VI_TVMODE_DEBUG_PAL_INT:
    return &timing[2];
  case VI_TVMODE_DEBUG_PAL_DS:
    return &timing[3];

  case VI_TVMODE_GCA_INT:
    return &timing[8];
  case VI_TVMODE_GCA_PROG:
    return &timing[9];
  }

  return nullptr;
#else
    val = tm->bs4 | (tm->be4 << 5);
    regs[12] = val;
    changed |= (1LL << (63-12));
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	800D0B84
 * Size:	000200
 */
void __VIInit(VITVMode mode) {
  VITimingInfo* tm;
  u32 nonInter;
  vu32 a;
  u32 tv, tvForReg;
#else
static void setScalingRegs(u16 panSizeX, u16 dispSizeX, BOOL threeD) {
    u32 scale;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  u16 hct, vct;

  nonInter = mode & 2;
  tv = (u32)mode >> 2;

  *(u32*)OSPhysicalToCached(0xCC) = tv;

  tm = getTiming(mode);

  __VIRegs[VI_DISP_CONFIG] = 2;
  for (a = 0; a < 1000; a++) {
    ;
  }

  __VIRegs[VI_DISP_CONFIG] = 0;

  __VIRegs[VI_HORIZ_TIMING_0U] = tm->hlw << 0;
  __VIRegs[VI_HORIZ_TIMING_0L] = (tm->hce << 0) | (tm->hcs << 8);

  __VIRegs[VI_HORIZ_TIMING_1U] = (tm->hsy << 0) | ((tm->hbe640 & ((1 << 9) - 1)) << 7);
  __VIRegs[VI_HORIZ_TIMING_1L] = ((tm->hbe640 >> 9) << 0) | (tm->hbs640 << 1);

  __VIRegs[VI_VERT_TIMING] = (tm->equ << 0) | (0 << 4);

  __VIRegs[VI_VERT_TIMING_ODD_U] = (tm->prbOdd + tm->acv * 2 - 2) << 0;
  __VIRegs[VI_VERT_TIMING_ODD] = tm->psbOdd + 2 << 0;

  __VIRegs[VI_VERT_TIMING_EVEN_U] = (tm->prbEven + tm->acv * 2 - 2) << 0;
  __VIRegs[VI_VERT_TIMING_EVEN] = tm->psbEven + 2 << 0;

  __VIRegs[VI_BBI_ODD_U] = (tm->bs1 << 0) | (tm->be1 << 5);
  __VIRegs[VI_BBI_ODD] = (tm->bs3 << 0) | (tm->be3 << 5);

  __VIRegs[VI_BBI_EVEN_U] = (tm->bs2 << 0) | (tm->be2 << 5);
  __VIRegs[VI_BBI_EVEN] = (tm->bs4 << 0) | (tm->be4 << 5);

  __VIRegs[VI_HSW] = (40 << 0) | (40 << 8);

  __VIRegs[VI_DISP_INT_1U] = 1;
  __VIRegs[VI_DISP_INT_1] = (1 << 0) | (1 << 12) | (0 << 15);

  hct = (tm->hlw + 1);
  vct = (tm->numHalfLines / 2 + 1) | (1 << 12) | (0 << 15);
  __VIRegs[VI_DISP_INT_0U] = hct << 0;
  __VIRegs[VI_DISP_INT_0] = vct;

  if (mode != VI_TVMODE_NTSC_PROG && mode != VI_TVMODE_NTSC_3D && mode != VI_TVMODE_GCA_PROG) {
    __VIRegs[VI_DISP_CONFIG] =
        (1 << 0) | (0 << 1) | (nonInter << 2) | (0 << 3) | (0 << 4) | (0 << 6) | (tv << 8);
    __VIRegs[VI_CLOCK_SEL] = 0;

  } else {
    __VIRegs[VI_DISP_CONFIG] =
        (1 << 0) | (0 << 1) | (1 << 2) | (0 << 3) | (0 << 4) | (0 << 6) | (tv << 8);
    __VIRegs[VI_CLOCK_SEL] = 1;
  }
#else
    panSizeX = threeD ? (panSizeX << 1) : panSizeX;
    if (panSizeX < dispSizeX) {
        scale = (u32)(dispSizeX + (panSizeX << 8) - 1) / dispSizeX;
        regs[37] = scale | 0x1000;
        changed |= 0x04000000;
        regs[56] = (u32)panSizeX;
        changed |= 0x80;
    } else {
        regs[37] = 0x100;
        changed |= 0x04000000;
    }
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	........
 * Size:	000160
 */
static void AdjustPosition(u16 acv) {
  s32 coeff, frac;
#else
static void calcFbbs(u32 bufAddr, u16 panPosX, u16 panPosY, u8 wordPerLine, VIXFBMode xfbMode, u16 dispPosY, u32* tfbb, u32* bfbb) {
    u32 bytesPerLine;
    u32 xoffInWords;
    u32 tmp;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  HorVer.adjDispPosX = (u16)CLAMP((s16)HorVer.dispPosX + displayOffsetH, 0, 720 - HorVer.dispSizeX);

  coeff = (HorVer.xfbMode == VI_XFBMODE_SF) ? 2 : 1;
  frac = HorVer.dispPosY & 1;

  HorVer.adjDispPosY = (u16)MAX((s16)HorVer.dispPosY + displayOffsetV, frac);

  HorVer.adjDispSizeY =
      (u16)(HorVer.dispSizeY + MIN((s16)HorVer.dispPosY + displayOffsetV - frac, 0) -
            MAX((s16)HorVer.dispPosY + (s16)HorVer.dispSizeY + displayOffsetV -
                    ((s16)acv * 2 - frac),
                0));

  HorVer.adjPanPosY =
      (u16)(HorVer.panPosY - MIN((s16)HorVer.dispPosY + displayOffsetV - frac, 0) / coeff);

  HorVer.adjPanSizeY =
      (u16)(HorVer.panSizeY + MIN((s16)HorVer.dispPosY + displayOffsetV - frac, 0) / coeff -
            MAX((s16)HorVer.dispPosY + (s16)HorVer.dispSizeY + displayOffsetV -
                    ((s16)acv * 2 - frac),
                0) /
                coeff);
#else
    xoffInWords = (panPosX & ~0xF) >> 4;
    bytesPerLine = (wordPerLine & 0xFF) << 5;
    *tfbb = bufAddr + (xoffInWords << 5) + (bytesPerLine * panPosY);
    *bfbb = (xfbMode == VI_XFBMODE_SF) ? *tfbb : *tfbb + bytesPerLine;
    if (dispPosY % 2 == 1) {
        tmp = *tfbb;
        *tfbb = *bfbb;
        *bfbb = tmp;
    }
    *tfbb &= 0x3FFFFFFF;
    *bfbb &= 0x3FFFFFFF;
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	........
 * Size:	00003C
 */
static void ImportAdjustingValues(void) {
  displayOffsetH = __OSLockSram()->displayOffsetH;
  displayOffsetV = 0;
  __OSUnlockSram(FALSE);
#else
static void setFbbRegs(SomeVIStruct* HorVer, u32* tfbb, u32* bfbb, u32* rtfbb, u32* rbfbb) {
    u32 shifted;

    calcFbbs(HorVer->bufAddr, HorVer->PanPosX, HorVer->AdjustedPanPosY, HorVer->wordPerLine, HorVer->FBMode, HorVer->AdjustedDispPosY, tfbb, bfbb);
    if (HorVer->threeD) {
        calcFbbs(HorVer->rbufAddr, HorVer->PanPosX, HorVer->AdjustedPanPosY, HorVer->wordPerLine, HorVer->FBMode, HorVer->AdjustedDispPosY, rtfbb, rbfbb);
    }

    if (*tfbb < 0x01000000U && *bfbb < 0x01000000U && *rtfbb < 0x01000000U && *rbfbb < 0x01000000U) {
        shifted = 0;
    } else {
        shifted = 1;
    }

    if (shifted) {
        *tfbb >>= 5;
        *bfbb >>= 5;
        *rtfbb >>= 5;
        *rbfbb >>= 5;
    }

    regs[15] = (u16)*tfbb & 0xFFFF;
    MARK_CHANGED(15);
    regs[14] = (shifted << 12) | ((*tfbb >> 16) | (HorVer->xof << 8));
    MARK_CHANGED(14);
    regs[19] = (u16)*bfbb & 0xFFFF;
    MARK_CHANGED(19);
    regs[18] = (*bfbb >> 16);
    MARK_CHANGED(18);

    if (HorVer->threeD) {
        regs[17] = (u16)*rtfbb & 0xFFFF;
        MARK_CHANGED(17);
        regs[16] = *rtfbb >> 16;
        MARK_CHANGED(16);
        regs[21] = (u16)*rbfbb & 0xFFFF;
        MARK_CHANGED(21);
        regs[20] = *rbfbb >> 16;
        MARK_CHANGED(20);
    }
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	800D0D84
 * Size:	0004B0
 */
void VIInit(void) {
  u16 dspCfg;
  u32 value, tv, tvInBootrom;
#else
static void setHorizontalRegs(VITiming* tm, u16 dispPosX, u16 dispSizeX) {
    u32 hbe;
    u32 hbs;
    u32 hbeLo;
    u32 hbeHi;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  if (IsInitialized) {
    return;
  }

  OSRegisterVersion(__VIVersion);
  IsInitialized = TRUE;
  encoderType = 1;

  if (!(__VIRegs[VI_DISP_CONFIG] & 1)) {
    __VIInit(VI_TVMODE_NTSC_INT);
  }

  retraceCount = 0;
  changed = 0;
  shdwChanged = 0;
  changeMode = 0;
  shdwChangeMode = 0;
  flushFlag = 0;

  __VIRegs[VI_FCT_0U] = ((((taps[0])) << 0) | (((taps[1] & ((1 << (6)) - 1))) << 10));
  __VIRegs[VI_FCT_0] = ((((taps[1] >> 6)) << 0) | (((taps[2])) << 4));
  __VIRegs[VI_FCT_1U] = ((((taps[3])) << 0) | (((taps[4] & ((1 << (6)) - 1))) << 10));
  __VIRegs[VI_FCT_1] = ((((taps[4] >> 6)) << 0) | (((taps[5])) << 4));
  __VIRegs[VI_FCT_2U] = ((((taps[6])) << 0) | (((taps[7] & ((1 << (6)) - 1))) << 10));
  __VIRegs[VI_FCT_2] = ((((taps[7] >> 6)) << 0) | (((taps[8])) << 4));
  __VIRegs[VI_FCT_3U] = ((((taps[9])) << 0) | (((taps[10])) << 8));
  __VIRegs[VI_FCT_3] = ((((taps[11])) << 0) | (((taps[12])) << 8));
  __VIRegs[VI_FCT_4U] = ((((taps[13])) << 0) | (((taps[14])) << 8));
  __VIRegs[VI_FCT_4] = ((((taps[15])) << 0) | (((taps[16])) << 8));
  __VIRegs[VI_FCT_5U] = ((((taps[17])) << 0) | (((taps[18])) << 8));
  __VIRegs[VI_FCT_5] = ((((taps[19])) << 0) | (((taps[20])) << 8));
  __VIRegs[VI_FCT_6U] = ((((taps[21])) << 0) | (((taps[22])) << 8));
  __VIRegs[VI_FCT_6] = ((((taps[23])) << 0) | (((taps[24])) << 8));

  __VIRegs[VI_WIDTH] = 640;
  ImportAdjustingValues();
  tvInBootrom = *(u32*)OSPhysicalToCached(0xCC);
  dspCfg = __VIRegs[VI_DISP_CONFIG];

  HorVer.nonInter = ((((u32)(dspCfg)) >> 2 & 0x00000001));
  HorVer.tv = ((((u32)(dspCfg)) & 0x00000300) >> 8);

  if ((tvInBootrom == VI_PAL) && (HorVer.tv == VI_NTSC)) {
    HorVer.tv = VI_EURGB60;
  }

  tv = (HorVer.tv == VI_DEBUG) ? VI_NTSC : HorVer.tv;
  HorVer.timing = getTiming((VITVMode)VI_TVMODE(tv, HorVer.nonInter));
  regs[VI_DISP_CONFIG] = dspCfg;

  CurrTiming = HorVer.timing;
  CurrTvMode = HorVer.tv;

  HorVer.dispSizeX = 640;
  HorVer.dispSizeY = (u16)(CurrTiming->acv * 2);
  HorVer.dispPosX = (u16)((720 - HorVer.dispSizeX) / 2);
  HorVer.dispPosY = 0;

  AdjustPosition(CurrTiming->acv);

  HorVer.fbSizeX = 640;
  HorVer.fbSizeY = (u16)(CurrTiming->acv * 2);
  HorVer.panPosX = 0;
  HorVer.panPosY = 0;
  HorVer.panSizeX = 640;
  HorVer.panSizeY = (u16)(CurrTiming->acv * 2);
  HorVer.xfbMode = VI_XFBMODE_SF;
  HorVer.wordPerLine = 40;
  HorVer.std = 40;
  HorVer.wpl = 40;
  HorVer.xof = 0;
  HorVer.isBlack = TRUE;
  HorVer.is3D = FALSE;

  OSInitThreadQueue(&retraceQueue);

  value = __VIRegs[VI_DISP_INT_0];
  value = (((u32)(value)) & ~0x00008000) | (((0)) << 15);
  __VIRegs[VI_DISP_INT_0] = value;

  value = __VIRegs[VI_DISP_INT_1];
  value = (((u32)(value)) & ~0x00008000) | (((0)) << 15);
  __VIRegs[VI_DISP_INT_1] = value;

  PreCB = nullptr;
  PostCB = nullptr;

  __OSSetInterruptHandler(24, __VIRetraceHandler);
  __OSUnmaskInterrupts((0x80000000u >> (24)));
#else
    regs[3] = (u16)(u32)tm->hlw;
    MARK_CHANGED(3);
    regs[2] = tm->hce | (tm->hcs << 8);
    MARK_CHANGED(2);
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    if (HorVer.tv == 8) {
        hbe = tm->hbe640 + 172;
        hbs = tm->hbs640;
    } else {
        hbe = tm->hbe640 - 40 + dispPosX;
        hbs = tm->hbs640 + 40 + dispPosX - (720 - dispSizeX);
    }
#else
    hbe = tm->hbe640 - 40 + dispPosX;
    hbs = tm->hbs640 + 40 + dispPosX - (720 - dispSizeX);
#endif
    hbeLo = hbe & 0x1FF;
    hbeHi = hbe >> 9;
    regs[5] = tm->hsy | (hbeLo << 7);
    MARK_CHANGED(5);
    regs[4] = hbeHi | (hbs * 2);
    MARK_CHANGED(4);
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	800D1234
 * Size:	000054
 */
void VIWaitForRetrace(void) {
  int interrupt;
  u32 startCount;
#else
static void setVerticalRegs(u16 dispPosY, u16 dispSizeY, u8 equ, u16 acv, u16 prbOdd, u16 prbEven, u16 psbOdd, u16 psbEven, BOOL black) {
    u16 actualPrbOdd;
    u16 actualPrbEven;
    u16 actualPsbOdd;
    u16 actualPsbEven;
    u16 actualAcv;
    u16 c;
    u16 d;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  interrupt = OSDisableInterrupts();
  startCount = retraceCount;
  do {
    OSSleepThread(&retraceQueue);
  } while (startCount == retraceCount);
  OSRestoreInterrupts(interrupt);
#else
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    if (HorVer.nonInter == 2 || HorVer.nonInter == 3) {
#else
    if (regs[54] & 1) {
#endif
        c = 1;
        d = 2;
    } else {
        c = 2;
        d = 1;
    }

    if ((dispPosY % 2) == 0) {
        actualPrbOdd = prbOdd + (d * dispPosY);
        actualPsbOdd = psbOdd + (d * (((c * acv) - dispSizeY) - dispPosY));
        actualPrbEven = prbEven + (d * dispPosY);
        actualPsbEven = psbEven + (d * (((c * acv) - dispSizeY) - dispPosY));
    } else {
        actualPrbOdd = prbEven + (d * dispPosY);
        actualPsbOdd = psbEven + (d * (((c * acv) - dispSizeY) - dispPosY));
        actualPrbEven = prbOdd + (d * dispPosY);
        actualPsbEven = psbOdd + (d * (((c * acv) - dispSizeY) - dispPosY));
    }

    actualAcv = dispSizeY / c;

    if (black) {
        actualPrbOdd += 2 * actualAcv - 2;
        actualPsbOdd += 2;
        actualPrbEven += 2 * actualAcv - 2;
        actualPsbEven += 2;
        actualAcv = 0;
    }

    regs[0] = equ | (actualAcv << 4);
    MARK_CHANGED(0);
    regs[7] = (u16)(u32)actualPrbOdd;
    MARK_CHANGED(7);
    regs[6] = (u16)(u32)actualPsbOdd;
    MARK_CHANGED(6);
    regs[9] = (u16)(u32)actualPrbEven;
    MARK_CHANGED(9);
    regs[8] = (u16)(u32)actualPsbEven;
    MARK_CHANGED(8);
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	........
 * Size:	00007C
 */
static void setInterruptRegs(VITimingInfo* tm) {
  u16 vct, hct, borrow;
#else
static void PrintDebugPalCaution(void) {
    static u32 message;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  vct = (u16)(tm->numHalfLines / 2);
  borrow = (u16)(tm->numHalfLines % 2);
  hct = (u16)((borrow) ? tm->hlw : (u16)0);

  vct++;
  hct++;

  regs[VI_DISP_INT_0U] = (u16)hct;
  changed |= VI_BITMASK(VI_DISP_INT_0U);

  regs[VI_DISP_INT_0] = (u16)((((u32)(vct))) | (((u32)(1)) << 12) | (((u32)(0)) << 15));
  changed |= VI_BITMASK(VI_DISP_INT_0);
#else
    if (message == 0) {
        message = 1;
        OSReport("***************************************\n");
        OSReport(" ! ! ! C A U T I O N ! ! !             \n");
        OSReport("This TV format \"DEBUG_PAL\" is only for \n");
        OSReport("temporary solution until PAL DAC board \n");
        OSReport("is available. Please do NOT use this   \n");
        OSReport("mode in real games!!!                  \n");
        OSReport("***************************************\n");
    }
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	........
 * Size:	000098
 */
static void setPicConfig(u16 fbSizeX, VIXFBMode xfbMode, u16 panPosX, u16 panSizeX, u8* wordPerLine,
                         u8* std, u8* wpl, u8* xof) {
  *wordPerLine = (u8)((fbSizeX + 15) / 16);
  *std = (u8)((xfbMode == VI_XFBMODE_SF) ? *wordPerLine : (u8)(2 * *wordPerLine));
  *xof = (u8)(panPosX % 16);
  *wpl = (u8)((*xof + panSizeX + 15) / 16);
#else
void VIConfigure(const GXRenderModeObj* rm) {
    VITiming* tm;
    u32 regDspCfg;
    u32 regClksel;
    BOOL enabled;
    u32 newNonInter;
    u32 tvInBootrom;
    u32 tvInGame;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  regs[VI_HSW] = (u16)((((u32)(*std))) | (((u32)(*wpl)) << 8));
  changed |= VI_BITMASK(VI_HSW);
}
#else
    enabled = OSDisableInterrupts();
    newNonInter = rm->viTVmode & 3;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	........
 * Size:	0000BC
 */
static void setBBIntervalRegs(VITimingInfo* tm) {
  u16 val;
#else
    if (HorVer.nonInter != newNonInter) {
        changeMode = 1;
        HorVer.nonInter = newNonInter;
    }
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  val = (u16)((((u32)(tm->bs1))) | (((u32)(tm->be1)) << 5));
  regs[VI_BBI_ODD_U] = val;
  changed |= VI_BITMASK(VI_BBI_ODD_U);
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  val = (u16)((((u32)(tm->bs3))) | (((u32)(tm->be3)) << 5));
  regs[VI_BBI_ODD] = val;
  changed |= VI_BITMASK(VI_BBI_ODD);
#else
    tvInGame = (u32)rm->viTVmode >> 2;
    tvInBootrom = *(u32*)OSPhysicalToCached(0xCC);
    
    if (tvInGame == VI_DEBUG_PAL) {
        PrintDebugPalCaution();
    }
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  val = (u16)((((u32)(tm->bs2))) | (((u32)(tm->be2)) << 5));
  regs[VI_BBI_EVEN_U] = val;
  changed |= VI_BITMASK(VI_BBI_EVEN_U);
#else
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    if ((tvInBootrom != VI_PAL && tvInBootrom != VI_EURGB60 &&
         (tvInGame == VI_PAL || tvInGame == VI_EURGB60)) ||
        ((tvInBootrom == VI_PAL || tvInBootrom == VI_EURGB60) &&
         (tvInGame != VI_PAL && tvInGame != VI_EURGB60))) {
        OSPanic(__FILE__, 2647,
                "VIConfigure(): Tried to change mode from (%d) to (%d), which is forbidden\n",
                tvInBootrom, tvInGame);
    }
#else
    switch (tvInBootrom) {
    case VI_MPAL:
    case VI_NTSC:
    case 6:
    case 7:
        if (tvInGame == VI_NTSC || tvInGame == VI_MPAL || tvInGame == 6 || tvInGame == 7) {
            break;
        }
        goto panic;
    case VI_PAL:
    case VI_EURGB60:
        if (tvInGame == VI_PAL || tvInGame == VI_EURGB60) {
            break;
        }
    default:
    panic:
        OSPanic(__FILE__, 1979,
                "VIConfigure(): Tried to change mode from (%d) to (%d), which is forbidden\n",
                tvInBootrom, tvInGame);
    }
#endif
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  val = (u16)((((u32)(tm->bs4))) | (((u32)(tm->be4)) << 5));
  regs[VI_BBI_EVEN] = val;
  changed |= VI_BITMASK(VI_BBI_EVEN);
}
#else
    if ((tvInGame == VI_NTSC) || (tvInGame == VI_MPAL)) {
        HorVer.tv = tvInBootrom;
    } else {
        HorVer.tv = tvInGame;
    }
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	........
 * Size:	00009C
 */
static void setScalingRegs(u16 panSizeX, u16 dispSizeX, BOOL is3D) {
  u32 scale;
#else
    HorVer.DispPosX  = rm->viXOrigin;
    HorVer.DispPosY  = (HorVer.nonInter == 1) ? (u16)(rm->viYOrigin * 2) : rm->viYOrigin;
    HorVer.DispSizeX = rm->viWidth;
    HorVer.FBSizeX   = rm->fbWidth;
    HorVer.FBSizeY   = rm->xfbHeight;
    HorVer.FBMode    = rm->xFBmode;
    HorVer.PanSizeX  = HorVer.FBSizeX;
    HorVer.PanSizeY  = HorVer.FBSizeY;
    HorVer.PanPosX   = 0;
    HorVer.PanPosY   = 0;
    HorVer.DispSizeY = (HorVer.nonInter == 2)           ? HorVer.PanSizeY :
                       (HorVer.nonInter == 3)           ? HorVer.PanSizeY :
                       (HorVer.FBMode == VI_XFBMODE_SF) ? (u16)(HorVer.PanSizeY * 2) :
                                                          HorVer.PanSizeY;
    HorVer.threeD = (HorVer.nonInter == 3) ? TRUE : FALSE;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  panSizeX = (u16)(is3D ? panSizeX * 2 : panSizeX);
#else
    tm = getTiming((HorVer.tv << 2) + HorVer.nonInter);
    HorVer.timing = tm;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  if (panSizeX < dispSizeX) {
    scale = (256 * (u32)panSizeX + (u32)dispSizeX - 1) / (u32)dispSizeX;
#else
    AdjustPosition(tm->acv);
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
    regs[VI_HSR] = (u16)((((u32)(scale))) | (((u32)(1)) << 12));
    changed |= VI_BITMASK(VI_HSR);
#else
#if SDK_VERSION < SDK_VERSION_RVL_2008_01
    if (encoderType == 0) {
        HorVer.tv = 3;
    }
#endif
    setInterruptRegs(tm);
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
    regs[VI_WIDTH] = (u16)((((u32)(panSizeX))));
    changed |= VI_BITMASK(VI_WIDTH);
  } else {
    regs[VI_HSR] = (u16)((((u32)(256))) | (((u32)(0)) << 12));
    changed |= VI_BITMASK(VI_HSR);
  }
}
#else
    regDspCfg = regs[1];
    regClksel = regs[54];
    if (HorVer.nonInter == VI_PROGRESSIVE || HorVer.nonInter == 3) {
        regDspCfg = (((u32)(regDspCfg)) & ~0x00000004) | (((u32)(1)) << 2);
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
        if (HorVer.tv == 8) {
            regClksel = (((u32)(regClksel)) & ~0x00000001) | (((u32)(0)) << 0);
        } else {
            regClksel = (((u32)(regClksel)) & ~0x00000001) | (((u32)(1)) << 0);
        }
#else
        regClksel = (((u32)(regClksel)) & ~0x00000001) | (((u32)(1)) << 0);
#endif
    } else {
        regDspCfg = (((u32)(regDspCfg)) & ~0x00000004) | (((u32)(HorVer.nonInter & 1)) << 2);
        regClksel = (((u32)(regClksel)) & ~0x00000001);
    }
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	........
 * Size:	000080
 */
static void calcFbbs(u32 bufAddr, u16 panPosX, u16 panPosY, u8 wordPerLine, VIXFBMode xfbMode,
                     u16 dispPosY, u32* tfbb, u32* bfbb) {
  u32 bytesPerLine, xoffInWords;
  xoffInWords = (u32)panPosX / 16;
  bytesPerLine = (u32)wordPerLine * 32;
#else
    regDspCfg = (((u32)(regDspCfg)) & ~0x00000008) | (((u32)(HorVer.threeD)) << 3);
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  *tfbb = bufAddr + xoffInWords * 32 + bytesPerLine * panPosY;
  *bfbb = (xfbMode == VI_XFBMODE_SF) ? *tfbb : (*tfbb + bytesPerLine);
#else
    if ((HorVer.tv == VI_PAL) || (HorVer.tv == VI_MPAL) || (HorVer.tv == 3)) {
        regDspCfg = (((u32)(regDspCfg)) & ~0x00000300) | (((u32)(HorVer.tv)) << 8);
    } else {
        regDspCfg = (((u32)(regDspCfg)) & ~0x00000300);
    }
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  if (dispPosY % 2 == 1) {
    u32 tmp = *tfbb;
    *tfbb = *bfbb;
    *bfbb = tmp;
  }
#else
    regs[1] = regDspCfg;
    regs[54] = (u16)regClksel;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  *tfbb = ToPhysical(*tfbb);
  *bfbb = ToPhysical(*bfbb);
}
#else
    MARK_CHANGED(1);    
    MARK_CHANGED(54);
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
/*
 * --INFO--
 * Address:	800D1288
 * Size:	0002D4
 */
static void setFbbRegs(VIPositionInfo* hv, u32* tfbb, u32* bfbb, u32* rtfbb, u32* rbfbb) {
  u32 shifted;
  calcFbbs(hv->bufAddr, hv->panPosX, hv->adjPanPosY, hv->wordPerLine, hv->xfbMode, hv->adjDispPosY,
           tfbb, bfbb);

  if (hv->is3D) {
    calcFbbs(hv->rbufAddr, hv->panPosX, hv->adjPanPosY, hv->wordPerLine, hv->xfbMode,
             hv->adjDispPosY, rtfbb, rbfbb);
  }

  if (IS_LOWER_16MB(*tfbb) && IS_LOWER_16MB(*bfbb) && IS_LOWER_16MB(*rtfbb) &&
      IS_LOWER_16MB(*rbfbb)) {
    shifted = 0;
  } else {
    shifted = 1;
  }

  if (shifted) {
    *tfbb >>= 5;
    *bfbb >>= 5;
    *rtfbb >>= 5;
    *rbfbb >>= 5;
  }

  regs[VI_TOP_FIELD_BASE_LEFT_U] = (u16)(*tfbb & 0xFFFF);
  changed |= VI_BITMASK(VI_TOP_FIELD_BASE_LEFT_U);

  regs[VI_TOP_FIELD_BASE_LEFT] = (u16)((((*tfbb >> 16))) | hv->xof << 8 | shifted << 12);
  changed |= VI_BITMASK(VI_TOP_FIELD_BASE_LEFT);

  regs[VI_BTTM_FIELD_BASE_LEFT_U] = (u16)(*bfbb & 0xFFFF);
  changed |= VI_BITMASK(VI_BTTM_FIELD_BASE_LEFT_U);

  regs[VI_BTTM_FIELD_BASE_LEFT] = (u16)(*bfbb >> 16);
  changed |= VI_BITMASK(VI_BTTM_FIELD_BASE_LEFT);

  if (hv->is3D) {
    regs[VI_TOP_FIELD_BASE_RIGHT_U] = *rtfbb & 0xffff;
    changed |= VI_BITMASK(VI_TOP_FIELD_BASE_RIGHT_U);

    regs[VI_TOP_FIELD_BASE_RIGHT] = *rtfbb >> 16;
    changed |= VI_BITMASK(VI_TOP_FIELD_BASE_RIGHT);

    regs[VI_BTTM_FIELD_BASE_RIGHT_U] = *rbfbb & 0xFFFF;
    changed |= VI_BITMASK(VI_BTTM_FIELD_BASE_RIGHT_U);

    regs[VI_BTTM_FIELD_BASE_RIGHT] = *rbfbb >> 16;
    changed |= VI_BITMASK(VI_BTTM_FIELD_BASE_RIGHT);
  }
}

/*
 * --INFO--
 * Address:	........
 * Size:	0000CC
 */
static void setHorizontalRegs(VITimingInfo* tm, u16 dispPosX, u16 dispSizeX) {
  u32 hbe, hbs, hbeLo, hbeHi;

  regs[VI_HORIZ_TIMING_0U] = (u16)tm->hlw;
  changed |= VI_BITMASK(VI_HORIZ_TIMING_0U);

  regs[VI_HORIZ_TIMING_0L] = (u16)(tm->hce | tm->hcs << 8);
  changed |= VI_BITMASK(VI_HORIZ_TIMING_0L);

  hbe = (u32)(tm->hbe640 - 40 + dispPosX);
  hbs = (u32)(tm->hbs640 + 40 + dispPosX - (720 - dispSizeX));

  hbeLo = hbe & ONES(9);
  hbeHi = hbe >> 9;

  regs[VI_HORIZ_TIMING_1U] = (u16)(tm->hsy | hbeLo << 7);
  changed |= VI_BITMASK(VI_HORIZ_TIMING_1U);

  regs[VI_HORIZ_TIMING_1L] = (u16)(hbeHi | hbs << 1);
  changed |= VI_BITMASK(VI_HORIZ_TIMING_1L);
}

/*
 * --INFO--
 * Address:	800D155C
 * Size:	0001A0
 */
static void setVerticalRegs(u16 dispPosY, u16 dispSizeY, u8 equ, u16 acv, u16 prbOdd, u16 prbEven,
                            u16 psbOdd, u16 psbEven, BOOL black) {
  u16 actualPrbOdd, actualPrbEven, actualPsbOdd, actualPsbEven, actualAcv, c, d;

  if (regs[VI_CLOCK_SEL] & 1) {
    c = 1;
    d = 2;
  } else {
    c = 2;
    d = 1;
  }

  if (dispPosY % 2 == 0) {
    actualPrbOdd = (u16)(prbOdd + d * dispPosY);
    actualPsbOdd = (u16)(psbOdd + d * ((c * acv - dispSizeY) - dispPosY));
    actualPrbEven = (u16)(prbEven + d * dispPosY);
    actualPsbEven = (u16)(psbEven + d * ((c * acv - dispSizeY) - dispPosY));
  } else {
    actualPrbOdd = (u16)(prbEven + d * dispPosY);
    actualPsbOdd = (u16)(psbEven + d * ((c * acv - dispSizeY) - dispPosY));
    actualPrbEven = (u16)(prbOdd + d * dispPosY);
    actualPsbEven = (u16)(psbOdd + d * ((c * acv - dispSizeY) - dispPosY));
  }

  actualAcv = (u16)(dispSizeY / c);

  if (black) {
    actualPrbOdd += 2 * actualAcv - 2;
    actualPsbOdd += 2;
    actualPrbEven += 2 * actualAcv - 2;
    actualPsbEven += 2;
    actualAcv = 0;
  }

  regs[VI_VERT_TIMING] = (u16)(equ | actualAcv << 4);
  changed |= VI_BITMASK(VI_VERT_TIMING);

  regs[VI_VERT_TIMING_ODD_U] = (u16)actualPrbOdd << 0;
  changed |= VI_BITMASK(VI_VERT_TIMING_ODD_U);

  regs[VI_VERT_TIMING_ODD] = (u16)actualPsbOdd << 0;
  changed |= VI_BITMASK(VI_VERT_TIMING_ODD);

  regs[VI_VERT_TIMING_EVEN_U] = (u16)actualPrbEven << 0;
  changed |= VI_BITMASK(VI_VERT_TIMING_EVEN_U);

  regs[VI_VERT_TIMING_EVEN] = (u16)actualPsbEven << 0;
  changed |= VI_BITMASK(VI_VERT_TIMING_EVEN);
}

/*
 * --INFO--
 * Address:	........
 * Size:	000094
 */
static void PrintDebugPalCaution(void) {
  static u32 message = 0;

  if (message == 0) {
    message = 1;
    OSReport("***************************************\n");
    OSReport(" ! ! ! C A U T I O N ! ! !             \n");
    OSReport("This TV format \"DEBUG_PAL\" is only for \n");
    OSReport("temporary solution until PAL DAC board \n");
    OSReport("is available. Please do NOT use this   \n");
    OSReport("mode in real games!!!                  \n");
    OSReport("***************************************\n");
  }
}

/*
 * --INFO--
 * Address:	800D16FC
 * Size:	000828
 */
void VIConfigure(const GXRenderModeObj* obj) {
  VITimingInfo* tm;
  u32 regDspCfg;
  BOOL enabled;
  u32 newNonInter, tvInBootrom, tvInGame;

  enabled = OSDisableInterrupts();
  newNonInter = (u32)obj->viTVmode & 3;

  if (HorVer.nonInter != newNonInter) {
    changeMode = 1;
    HorVer.nonInter = newNonInter;
  }

  tvInGame = (u32)obj->viTVmode >> 2;
  tvInBootrom = *(u32*)OSPhysicalToCached(0xCC);

  if (tvInGame == VI_DEBUG_PAL) {
    PrintDebugPalCaution();
  }

  switch (tvInBootrom) {
  case VI_MPAL:
  case VI_NTSC:
  case VI_GCA:
    if (tvInGame == VI_NTSC || tvInGame == VI_MPAL || tvInGame == VI_GCA) {
      break;
    }
    goto panic;
  case VI_PAL:
  case VI_EURGB60:
    if (tvInGame == VI_PAL || tvInGame == VI_EURGB60) {
      break;
    }
  default:
  panic:
    OSPanic(__FILE__, 1884,
            "VIConfigure(): Tried to change mode from (%d) to (%d), which is forbidden\n",
            tvInBootrom, tvInGame);
  }
  if ((tvInGame == VI_NTSC) || (tvInGame == VI_MPAL)) {
    HorVer.tv = tvInBootrom;
  } else {
    HorVer.tv = tvInGame;
  }

  HorVer.dispPosX = obj->viXOrigin;
  HorVer.dispPosY =
      (u16)((HorVer.nonInter == VI_NON_INTERLACE) ? (u16)(obj->viYOrigin * 2) : obj->viYOrigin);
  HorVer.dispSizeX = obj->viWidth;
  HorVer.fbSizeX = obj->fbWidth;
  HorVer.fbSizeY = obj->xfbHeight;
  HorVer.xfbMode = obj->xFBmode;
  HorVer.panSizeX = HorVer.fbSizeX;
  HorVer.panSizeY = HorVer.fbSizeY;
  HorVer.panPosX = 0;
  HorVer.panPosY = 0;

  HorVer.dispSizeY = (u16)((HorVer.nonInter == VI_PROGRESSIVE) ? HorVer.panSizeY
                           : (HorVer.nonInter == VI_3D)        ? HorVer.panSizeY
                           : (HorVer.xfbMode == VI_XFBMODE_SF) ? (u16)(2 * HorVer.panSizeY)
                                                               : HorVer.panSizeY);

  HorVer.is3D = (HorVer.nonInter == VI_3D) ? TRUE : FALSE;

  tm = getTiming((VITVMode)VI_TVMODE(HorVer.tv, HorVer.nonInter));
  HorVer.timing = tm;

  AdjustPosition(tm->acv);
  if (encoderType == 0) {
    HorVer.tv = VI_DEBUG;
  }
  setInterruptRegs(tm);

  regDspCfg = regs[VI_DISP_CONFIG];

  if ((HorVer.nonInter == VI_PROGRESSIVE) || (HorVer.nonInter == VI_3D)) {
    regDspCfg = (((u32)(regDspCfg)) & ~0x00000004) | (((u32)(1)) << 2);
  } else {
    regDspCfg = (((u32)(regDspCfg)) & ~0x00000004) | (((u32)(HorVer.nonInter & 1)) << 2);
  }

  regDspCfg = (((u32)(regDspCfg)) & ~0x00000008) | (((u32)(HorVer.is3D)) << 3);

  if ((HorVer.tv == VI_DEBUG_PAL) || (HorVer.tv == VI_EURGB60) || (HorVer.tv == VI_GCA)) {
    regDspCfg = (((u32)(regDspCfg)) & ~0x00000300) | (((u32)(0)) << 8);
  } else {
    regDspCfg = (((u32)(regDspCfg)) & ~0x00000300) | (((u32)(HorVer.tv)) << 8);
  }

  regs[VI_DISP_CONFIG] = (u16)regDspCfg;
  changed |= VI_BITMASK(0x01);

  regDspCfg = regs[VI_CLOCK_SEL];
  if (obj->viTVmode == VI_TVMODE_NTSC_PROG || obj->viTVmode == VI_TVMODE_NTSC_3D ||
      obj->viTVmode == VI_TVMODE_GCA_PROG) {
    regDspCfg = (u32)(regDspCfg & ~0x1) | 1;
  } else {
    regDspCfg = (u32)(regDspCfg & ~0x1);
  }

  regs[VI_CLOCK_SEL] = (u16)regDspCfg;

  changed |= 0x200;

  setScalingRegs(HorVer.panSizeX, HorVer.dispSizeX, HorVer.is3D);
  setHorizontalRegs(tm, HorVer.adjDispPosX, HorVer.dispSizeX);
  setBBIntervalRegs(tm);
  setPicConfig(HorVer.fbSizeX, HorVer.xfbMode, HorVer.panPosX, HorVer.panSizeX, &HorVer.wordPerLine,
               &HorVer.std, &HorVer.wpl, &HorVer.xof);

  if (FBSet) {
    setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
  }

  setVerticalRegs(HorVer.adjDispPosY, HorVer.adjDispSizeY, tm->equ, tm->acv, tm->prbOdd,
                  tm->prbEven, tm->psbOdd, tm->psbEven, HorVer.isBlack);
  OSRestoreInterrupts(enabled);
}

void VIConfigurePan(u16 xOrg, u16 yOrg, u16 width, u16 height)
{
    BOOL enabled;
    VITimingInfo *tm;

// #if DEBUG
//     ASSERTMSGLINEV(0x69C, (xOrg & 1) == 0,
//         "VIConfigurePan(): Odd number(%d) is specified to xOrg\n",
//         xOrg);
//     if (HorVer.FBMode == VI_XFBMODE_DF) {
//         ASSERTMSGLINEV(0x6A1, (height & 1) == 0,
//             "VIConfigurePan(): Odd number(%d) is specified to height when DF XFB mode\n",
//             height);
//     }
// #endif
    enabled = OSDisableInterrupts();
    HorVer.panPosX   = xOrg;
    HorVer.panPosY   = yOrg;
    HorVer.panSizeX  = width;
    HorVer.panSizeY  = height;
    HorVer.dispSizeY = (HorVer.nonInter == 2) ? HorVer.panSizeY
                     : (HorVer.nonInter == 3) ? HorVer.panSizeY : (HorVer.xfbMode == VI_XFBMODE_SF) ? (u16)(HorVer.panSizeY * 2)
                     : HorVer.panSizeY;
    tm = HorVer.timing;
    AdjustPosition(tm->acv);
    setScalingRegs(HorVer.panSizeX, HorVer.dispSizeX, HorVer.is3D);
    setPicConfig(HorVer.fbSizeX, HorVer.xfbMode, HorVer.panPosX, HorVer.panSizeX, &HorVer.wordPerLine, &HorVer.std, &HorVer.wpl, &HorVer.xof);
#else
    setScalingRegs(HorVer.PanSizeX, HorVer.DispSizeX, HorVer.threeD);
    setHorizontalRegs(tm, HorVer.AdjustedDispPosX, HorVer.DispSizeX);
    setBBIntervalRegs(tm);
    setPicConfig(HorVer.FBSizeX, HorVer.FBMode, HorVer.PanPosX, HorVer.PanSizeX, &HorVer.wordPerLine, &HorVer.std, &HorVer.wpl, &HorVer.xof);
#endif
    if (FBSet != 0) {
        setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
    }
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
    setVerticalRegs(HorVer.adjDispPosY, HorVer.dispSizeY, tm->equ, tm->acv, tm->prbOdd, tm->prbEven, tm->psbOdd, tm->psbEven, HorVer.isBlack);
#else
    setVerticalRegs(HorVer.AdjustedDispPosY, HorVer.AdjustedDispSizeY, tm->equ, tm->acv, tm->prbOdd, tm->prbEven, tm->psbOdd, tm->psbEven, HorVer.black);
    OSRestoreInterrupts(enabled);
}

void VIConfigurePan(u16 xOrg, u16 yOrg, u16 width, u16 height) {
    BOOL enabled;
    VITiming* tm;

    enabled = OSDisableInterrupts();
    HorVer.PanPosX   = xOrg;
    HorVer.PanPosY   = yOrg;
    HorVer.PanSizeX  = width;
    HorVer.PanSizeY  = height;
    HorVer.DispSizeY = (HorVer.nonInter == 2)           ? HorVer.PanSizeY :
                       (HorVer.nonInter == 3)           ? HorVer.PanSizeY :
                       (HorVer.FBMode == VI_XFBMODE_SF) ? (u16)(HorVer.PanSizeY * 2) :
                                                          HorVer.PanSizeY;
    tm = HorVer.timing;
    AdjustPosition(tm->acv);
    setScalingRegs(HorVer.PanSizeX, HorVer.DispSizeX, HorVer.threeD);
    setPicConfig(HorVer.FBSizeX, HorVer.FBMode, HorVer.PanPosX, HorVer.PanSizeX, &HorVer.wordPerLine, &HorVer.std, &HorVer.wpl, &HorVer.xof);
    if (FBSet != 0) {
        setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
    }
    setVerticalRegs(HorVer.AdjustedDispPosY, HorVer.DispSizeY, tm->equ, tm->acv, tm->prbOdd, tm->prbEven, tm->psbOdd, tm->psbEven, HorVer.black);
#endif
    OSRestoreInterrupts(enabled);
}

void VIFlush(void) {
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  BOOL enabled;
  s32 regIndex;
  u32 val; // for stack.
#else
    BOOL enabled;
    s32 regIndex;
#endif

    enabled = OSDisableInterrupts();
    shdwChangeMode |= changeMode;
    changeMode = 0;
    shdwChanged |= changed;

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  while (changed) {
    regIndex = cntlzd(changed);
    shdwRegs[regIndex] = regs[regIndex];
    changed &= ~VI_BITMASK(regIndex);
  }
#else
    while (changed != 0) {
        regIndex = cntlzd(changed);
        shdwRegs[regIndex] = regs[regIndex];
        changed &= ~((u64)1 << (63 - regIndex));
    }
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  flushFlag = 1;
  NextBufAddr = HorVer.bufAddr;
  OSRestoreInterrupts(enabled);
#else
    flushFlag = 1;
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    flushFlag3in1 = 1;
#endif
    NextBufAddr = HorVer.bufAddr;
    OSRestoreInterrupts(enabled);
#endif
}

void VISetNextFrameBuffer(void* fb) {
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  BOOL enabled = OSDisableInterrupts();
  HorVer.bufAddr = (u32)fb;
  FBSet = 1;
  setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
  OSRestoreInterrupts(enabled);
#else
    BOOL enabled;

    enabled = OSDisableInterrupts();
    HorVer.bufAddr = (u32)fb;
    FBSet = 1;
    setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
    OSRestoreInterrupts(enabled);
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
void* VIGetNextFrameBuffer() {
  // UNUSED FUNCTION
#else
void* VIGetNextFrameBuffer(void) {
    return *(void**)(&NextBufAddr);
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
void* VIGetCurrentFrameBuffer(void) { return (void*)CurrBufAddr; }
#else
void* VIGetCurrentFrameBuffer(void) {
    return *(void**)(&CurrBufAddr);
}
#endif

void VISetNextRightFrameBuffer(void* fb) {
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  // UNUSED FUNCTION
#else
    BOOL enabled;

    enabled = OSDisableInterrupts();
    HorVer.rbufAddr = (u32)fb;
    FBSet = 1;
    setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
    OSRestoreInterrupts(enabled);
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
void VISetBlack(BOOL isBlack) {
  int interrupt;
  VITimingInfo* tm;
#else
void VISetBlack(BOOL black) {
    BOOL enabled;
    VITiming* tm;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  interrupt = OSDisableInterrupts();
  HorVer.isBlack = isBlack;
  tm = HorVer.timing;
  setVerticalRegs(HorVer.adjDispPosY, HorVer.dispSizeY, tm->equ, tm->acv, tm->prbOdd, tm->prbEven,
                  tm->psbOdd, tm->psbEven, HorVer.isBlack);
  OSRestoreInterrupts(interrupt);
#else
    enabled = OSDisableInterrupts();
    HorVer.black = black;
    tm = HorVer.timing;
    setVerticalRegs(HorVer.AdjustedDispPosY, HorVer.DispSizeY, tm->equ, tm->acv, tm->prbOdd, tm->prbEven, tm->psbOdd, tm->psbEven, HorVer.black);
    OSRestoreInterrupts(enabled);
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
void VISet3D(void) {
  // UNUSED FUNCTION
#else
// NONMATCHING
void VISet3D(BOOL threeD) {
    BOOL enabled;
    u32 reg;

    enabled = OSDisableInterrupts();
    HorVer.threeD = threeD;
    reg = regs[1];
    SET_REG_FIELD(0x933, reg, 1, 3, HorVer.threeD);
    regs[1] = reg;
    MARK_CHANGED(1);
    setScalingRegs(HorVer.PanSizeX, HorVer.DispSizeX, HorVer.threeD);
    OSRestoreInterrupts(enabled);
#endif
}

u32 VIGetRetraceCount(void) {
    return retraceCount;
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
static u32 getCurrentHalfLine(void)
{
    u32 hcount;
    u32 vcount0;
    u32 vcount;
#else
static void GetCurrentDisplayPosition(u32* hct, u32* vct) {
    u32 hcount, vcount0, vcount;
    vcount = __VIRegs[VI_VERT_COUNT] & 0x7FF;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
    vcount = __VIRegs[22] & 0x7FF;
#endif
    do {
        vcount0 = vcount;
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
        hcount = __VIRegs[23] & 0x7FF;
        vcount = __VIRegs[22] & 0x7FF;
#else
        hcount = __VIRegs[VI_HORIZ_COUNT] & 0x7FF;
        vcount = __VIRegs[VI_VERT_COUNT] & 0x7FF;
#endif
    } while (vcount0 != vcount);
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
    return ((vcount - 1) * 2) + ((hcount - 1) / CurrTiming->hlw);
#else

    *hct = hcount;
    *vct = vcount;
#endif
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
static u32 getCurrentFieldEvenOdd() {
  return (getCurrentHalfLine() < CurrTiming->numHalfLines) ? 1 : 0;
#else
static u32 getCurrentHalfLine(void) {
    u32 hcount, vcount;
    GetCurrentDisplayPosition(&hcount, &vcount);

    return ((vcount - 1) << 1) + ((hcount - 1) / CurrTiming->hlw);
}

static u32 getCurrentFieldEvenOdd(void) {
    return (getCurrentHalfLine() < CurrTiming->nhlines) ? 1 : 0;
#endif
}

u32 VIGetNextField(void) {
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  u32 nextField;
  int interrupt;
#else
    s32 nextField;
    BOOL enabled;
    u8 unused[4];
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  interrupt = OSDisableInterrupts();
  nextField = getCurrentFieldEvenOdd() ^ 1;
  OSRestoreInterrupts(interrupt);
  return nextField ^ (HorVer.adjDispPosY & 1);
#else
    enabled = OSDisableInterrupts();
    nextField = getCurrentFieldEvenOdd() ^ 1;
    OSRestoreInterrupts(enabled);
    return nextField ^ (HorVer.AdjustedDispPosY & 1);
#endif
}

u32 VIGetCurrentLine(void) {
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  u32 line;
  VITimingInfo* tm;
  int interrupt;
#else
    u32 halfLine;
    VITiming* tm;
    BOOL enabled;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  tm = CurrTiming;
  interrupt = OSDisableInterrupts();
  line = getCurrentHalfLine();
  OSRestoreInterrupts(interrupt);

  if (line >= tm->numHalfLines) {
    line -= tm->numHalfLines;
  }

  return (line >> 1);
#else
    tm = CurrTiming;
    enabled = OSDisableInterrupts();
    halfLine = getCurrentHalfLine();
    OSRestoreInterrupts(enabled);
    if (halfLine >= tm->nhlines) {
        halfLine -= tm->nhlines;
    }
    return halfLine >> 1U;
#endif
}

u32 VIGetTvFormat(void) {
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  u32 fmt;
  int interrupt;
#else
    u32 format;
    BOOL enabled;
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  interrupt = OSDisableInterrupts();
#else
    enabled = OSDisableInterrupts();
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  switch (CurrTvMode) {
  case VI_NTSC:
  case VI_DEBUG:
  case VI_GCA:
    fmt = VI_NTSC;
    break;
  case VI_PAL:
  case VI_DEBUG_PAL:
    fmt = VI_PAL;
    break;
  case VI_EURGB60:
  case VI_MPAL:
    fmt = CurrTvMode;
    break;
  }

  OSRestoreInterrupts(interrupt);
  return fmt;
#else
    switch (CurrTvMode) {
    case VI_NTSC:
    case VI_DEBUG:
    case 6:
    case 7:
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
    case 8:
#endif
        format = VI_NTSC;
        break;
    case VI_PAL:
    case VI_DEBUG_PAL:
        format = VI_PAL;
        break;
    case VI_EURGB60:
    case VI_MPAL:
        format = CurrTvMode;
        break;
    default:
        ;
    }
    
    OSRestoreInterrupts(enabled);
    return format;
#endif
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
u32 VIGetScanMode(void) {
    u32 scanMode;
    BOOL enabled;

    enabled = OSDisableInterrupts();

    if ((((u32)__VIRegs[54] & 0x00000001) >> 0) == 1) {
        scanMode = VI_PROGRESSIVE;
    } else if ((((u32)__VIRegs[1] & 0x00000004) >> 2) == 0) {
        scanMode = VI_INTERLACE;
    } else {
        scanMode = VI_NON_INTERLACE;
    }

    OSRestoreInterrupts(enabled);
    return scanMode;
}
#else
// NONMATCHING
u32 VIGetScanMode(void) {
    u32 scanMode = 0;
    BOOL enabled = OSDisableInterrupts();

    if ((u32)(__VIRegs[54] & 1) == 1) {
        scanMode = 2;
    } else if (((__VIRegs[scanMode] >> 2) & 1) == 0) {
        scanMode = 0;
    } else {
        scanMode = 1;
    }
    
    OSRestoreInterrupts(enabled);
    return scanMode;
}
#endif

#endif
u32 VIGetDTVStatus(void) {
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  u32 stat;
  int interrupt;
#else
    u32 dtvStatus;
    BOOL enabled = OSDisableInterrupts();
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  interrupt = OSDisableInterrupts();
  stat = (__VIRegs[VI_DTV_STAT] & 3);
  OSRestoreInterrupts(interrupt);
  return (stat & 1);
#else
    dtvStatus = __VIRegs[55] & 3;
    OSRestoreInterrupts(enabled);
    return dtvStatus & 1;
#endif
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
void __VISetAdjustingValues(s16 x, s16 y) {
    BOOL enabled;
    VITiming* tm;

    enabled = OSDisableInterrupts();
    displayOffsetH = x;
    displayOffsetV = y;
    tm = HorVer.timing;
    AdjustPosition(tm->acv);
    setHorizontalRegs(tm, HorVer.AdjustedDispPosX, HorVer.DispSizeX);
    if (FBSet != 0) {
        setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
    }
    setVerticalRegs(HorVer.AdjustedDispPosY, HorVer.AdjustedDispSizeY, tm->equ, tm->acv, tm->prbOdd, tm->prbEven, tm->psbOdd, tm->psbEven, HorVer.black);
    OSRestoreInterrupts(enabled);
}

void __VIGetAdjustingValues(s16* x, s16* y) {
    BOOL enabled;

    enabled = OSDisableInterrupts();
    *x = displayOffsetH;
    *y = displayOffsetV;
    OSRestoreInterrupts(enabled);
}

// NONMATCHING
void __VIEnableRawPositionInterrupt(s16 x, s16 y, void (*callback)(s16, s16)) {
    BOOL enabled;
    u32 halfLine;
    u32 halfLineOff;

    enabled = OSDisableInterrupts();
    __VIRegs[29] = x + 1;
    __VIRegs[31] = x + 1;

    if (HorVer.nonInter == 0) {
        if (y & 1) {
            halfLineOff = CurrTiming->prbEven + ((CurrTiming->equ * 3) + CurrTiming->nhlines);
            __VIRegs[30] = (((halfLineOff / 2) + (y / 2)) + 1) | 0x1000;
        } else {
            halfLineOff = CurrTiming->prbOdd + (CurrTiming->equ * 3);
            __VIRegs[28] = (((halfLineOff / 2) + (y / 2)) + 1) | 0x1000;
        }
    } else if (HorVer.nonInter == 1) {
        halfLine = CurrTiming->prbOdd + ((CurrTiming->equ * 3)) + y;
        __VIRegs[28] = ((halfLine / 2) + 1) | 0x1000;
        __VIRegs[30] = (((halfLine + CurrTiming->nhlines) / 2) + 1) | 0x1000;
    } else if (HorVer.nonInter == 2) {
        halfLine = CurrTiming->prbOdd + ((CurrTiming->equ * 3)) + y;
        __VIRegs[28] = (halfLine + 1) | 0x1000;
        __VIRegs[30] = 0;
    }

    PositionCallback = callback;
    OSRestoreInterrupts(enabled);
}

void (*__VIDisableRawPositionInterrupt())(s16, s16) {
    BOOL enabled;
    void (*old)(s16, s16);

    enabled = OSDisableInterrupts();
    __VIRegs[28] = 0;
    __VIRegs[30] = 0;

    old = PositionCallback;
    PositionCallback = 0;
    OSRestoreInterrupts(enabled);
    return old;
}

void __VIDisplayPositionToXY(u32 hct, u32 vct, s16* x, s16* y) {
    u32 halfLine = ((vct - 1) << 1) + ((hct - 1) / CurrTiming->hlw);

    if (HorVer.nonInter == VI_INTERLACE) {
        if (halfLine < CurrTiming->nhlines) {
            if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbOdd) {
                *y = -1;
            } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbOdd) {
                *y = -1;
            } else {
                *y = (s16)((halfLine - CurrTiming->equ * 3 - CurrTiming->prbOdd) & ~1);
            }
        } else {
            halfLine -= CurrTiming->nhlines;

            if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbEven) {
                *y = -1;
            } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbEven) {
                *y = -1;
            } else {
                *y = (s16)(((halfLine - CurrTiming->equ * 3 - CurrTiming->prbEven) & ~1) + 1);
            }
        }
    } else if (HorVer.nonInter == VI_NON_INTERLACE) {
        if (halfLine >= CurrTiming->nhlines) {
            halfLine -= CurrTiming->nhlines;
        }

        if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbOdd) {
            *y = -1;
        } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbOdd) {
            *y = -1;
        } else {
            *y = (s16)((halfLine - CurrTiming->equ * 3 - CurrTiming->prbOdd) & ~1);
        }
    } else if (HorVer.nonInter == VI_PROGRESSIVE) {
        if (halfLine < CurrTiming->nhlines) {
            if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbOdd) {
                *y = -1;
            } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbOdd) {
                *y = -1;
            } else {
                *y = (s16)(halfLine - CurrTiming->equ * 3 - CurrTiming->prbOdd);
            }
        } else {
            halfLine -= CurrTiming->nhlines;

            if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbEven) {
                *y = -1;
            } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbEven) {
                *y = -1;
            } else
                *y = (s16)((halfLine - CurrTiming->equ * 3 - CurrTiming->prbEven) & ~1);
        }
    }

    *x = (s16)(hct - 1);
}

void __VIGetCurrentPosition(s16* x, s16* y) {
    u32 hcount, vcount;
    GetCurrentDisplayPosition(&hcount, &vcount);
    __VIDisplayPositionToXY(hcount, vcount, x, y);
}

// NONMATCHING
void __VISetLatchMode(u32 mode) {
    u32 reg;

    reg = __VIRegs[1];
    SET_REG_FIELD(0xB12, reg, 3, 6, mode & ~3);
    SET_REG_FIELD(0xB13, reg, 5, 8, mode & ~3);
    __VIRegs[1] = reg;
}

#pragma dont_inline on
// NONMATCHING
int __VIGetLatch0Position(s16* px, s16* py) {
    u32 hcount;
    u32 vcount;

    if ((__VIRegs[31] >> 0xF) & 1) {
        vcount = __VIRegs[32] & 0x7FF;
        hcount = __VIRegs[33] & 0x7FF;
        __VIRegs[32] = 0;
        __VIRegs[33] = 0;
        __VIDisplayPositionToXY(hcount, vcount, px, py);
        return 1;
    }

    *px = *py = -1;
    return 0;
}
#pragma dont_inline reset

#pragma dont_inline on
// NONMATCHING
int __VIGetLatch1Position(s16* px, s16* py) {
    u32 hcount;
    u32 vcount;

    if ((__VIRegs[31] >> 0xF) & 1) {
        vcount = __VIRegs[34] & 0x7FF;
        hcount = __VIRegs[35] & 0x7FF;
        __VIRegs[34] = 0;
        __VIRegs[35] = 0;
        __VIDisplayPositionToXY(hcount, vcount, px, py);
        return 1;
    }

    *px = *py = -1;
    return 0;
}
#pragma dont_inline reset

int __VIGetLatchPosition(u32 port, s16* px, s16* py) {
    if (port == 0) {
        return __VIGetLatch0Position(px, py);
    } else {
        return __VIGetLatch1Position(px, py);
    }
}

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
BOOL __VIResetDev0Idle(void) {
    __VIDimmingFlag_DEV_IDLE[0] = 0;
    return TRUE;
}

BOOL VIEnableDVDStopMotor(BOOL enable) {
    BOOL old = __VIDVDStopFlag_Enable;

    __VIDVDStopFlag_Enable = enable;
    return old;
}

BOOL VIEnableDimming(BOOL enable) {
    u8 value;
    BOOL old = __VIDimmingFlag_Enable;

    if (enable == TRUE) {
        value = SCGetScreenSaverMode();

        if (value == 0) {
            enable = FALSE;
        }
    }

    __VIDimmingFlag_Enable = enable;
    return old;
}

VITimeToDIM VISetTimeToDimming(VITimeToDIM time) {
    VITimeToDIM old_time = g_current_time_to_dim;

    g_current_time_to_dim = time;

    switch (VIGetTvFormat()) {
    case VI_PAL:
        switch (g_current_time_to_dim) {
        case VI_DM_10M:
            NEW_TIME_TO_DIMMING = 30000;
            break;
        case VI_DM_15M:
            NEW_TIME_TO_DIMMING = 45000;
            break;
        default:
            NEW_TIME_TO_DIMMING = 15000;
            break;
        }
        break;
    default:
        switch (g_current_time_to_dim) {
        case VI_DM_10M:
            NEW_TIME_TO_DIMMING = 36000;
            break;
        case VI_DM_15M:
            NEW_TIME_TO_DIMMING = 54000;
            break;
        default:
            NEW_TIME_TO_DIMMING = 18000;
            break;
        }
        break;
    }
    return old_time;
}

BOOL VIResetDimmingCount(void) {
    return __VIResetDev0Idle();
}

BOOL __VIResetRFIdle(void) {
    __VIDimmingFlag_RF_IDLE = 0;
    return TRUE;
}

BOOL __VIResetSIIdle(void) {
    __VIDimmingFlag_SI_IDLE = 0;
    return TRUE;
}
#endif

#endif