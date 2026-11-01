#include <SDKVersion.h>
#if SDK_VERSION < SDK_VERSION_G2ME01_00
#include "dolphin/gx.h"
#else
#include <stddef.h>
#include <dolphin/gx.h>
#include <dolphin/os.h>
#endif

#if SDK_VERSION < SDK_VERSION_G2ME01_00
#include "dolphin/gx/GXPriv.h"
#else
#include <dolphin/gx/__gx.h>
#endif

#if SDK_VERSION < SDK_VERSION_G2ME01_00
void __GXSetDirtyState(void) {
#define dState __GXData->dirtyState

  if (dState & 1) {
    __GXSetSUTexRegs();
  }
  if (dState & 2) {
    __GXUpdateBPMask();
  }
  if (dState & 4) {
    __GXSetGenMode();
  }
  if (dState & 8) {
    __GXSetVCD();
  }
  if (dState & 0x10) {
    __GXSetVAT();
  }
  if (dState & 0x18) {
    __GXCalculateVLim();
  }

  __GXData->dirtyState = 0;
}

void GXBegin(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts) {

  if (__GXData->dirtyState != 0) {
    __GXSetDirtyState();
  }

  if (*(u32 *)&__GXData->vNumNot == 0) { // checks both vNum and bpSentNot
    __GXSendFlushPrim();
  }
  GX_WRITE_U8(vtxfmt | type);
  GX_WRITE_U16(nverts);
}

void __GXSendFlushPrim(void) {
  u32 i;
  u32 numD = __GXData->vNum * __GXData->vLim;

  GX_WRITE_U8(0x98);
  GX_WRITE_U16(__GXData->vNum);
  for (i = 0; i < numD; i += 4) {
    GX_WRITE_U32(0);
  }
  __GXData->bpSentNot = 1;
}

void GXSetLineWidth(u8 width, GXTexOffset texOffsets) {

  SET_REG_FIELD(__GXData->lpSize, 8, 0, width);
  SET_REG_FIELD(__GXData->lpSize, 3, 16, texOffsets);
  GX_WRITE_RAS_REG(__GXData->lpSize);
  __GXData->bpSentNot = 0;
#else
#if SDK_VERSION >= SDK_VERSION_R32J01_00
static inline void __GXSetAmbMat(u32 dirty)
{
    if (dirty & 0x100) {
        GX_WRITE_XF_REG(0xA, __GXData->ambColor[0]);
    }
    if (dirty & 0x200) {
        GX_WRITE_XF_REG(0xB, __GXData->ambColor[1]);
    }
    if (dirty & 0x400) {
        GX_WRITE_XF_REG(0xC, __GXData->matColor[0]);
    }
    if (dirty & 0x800) {
        GX_WRITE_XF_REG(0xD, __GXData->matColor[1]);
    }
}

static inline void __GXSetLightChan(volatile s32 dirty)
{
    int i;
    u32 bits;
    u32 addr = 0x100E;

    if (dirty & 0x1000000) {
        u32 colors = (__GXData->genMode >> 4) & 7;
        GX_WRITE_XF_REG(9, colors);
    }

    bits = dirty;
    bits = (bits >> 12) & 0xF;
    i = 0;
    for (; bits != 0; i++, addr++, bits >>= 1) {
        if (bits & 1) {
            GX_WRITE_U8(0x10);
            GX_WRITE_U32(addr);
            GX_WRITE_U32(__GXData->colorControl[i]);
        }
    }
}

static inline void __GXSetTexGen(volatile s32 dirty)
{
    int i;
    u32 bits;
    u32 taddr = 0x1040;
    u32 dtaddr;

    if (dirty & 0x2000000) {
        u32 gens = __GXData->genMode & 0xF;
        GX_WRITE_XF_REG(0x3F, gens);
    }

    bits = dirty;
    bits = (bits >> 16) & 0xFF;
    i = 0;
    for (; bits != 0; taddr++, i++, bits >>= 1) {
        dtaddr = taddr + 0x10;
        if (bits & 1) {
            GX_WRITE_U8(0x10);
            GX_WRITE_U32(taddr);
            GX_WRITE_U32(__GXData->texRegs[i]);
            GX_WRITE_U8(0x10);
            GX_WRITE_U32(dtaddr);
            GX_WRITE_U32(__GXData->dualTexRegs[i]);
        }
    }
}

void __GXSetDirtyState(void)
{
    u32 temp;
    u32 dState = __GXData->dirtyState;

    if (dState & 1) {
        __GXSetSUTexRegs();
    }
    if (dState & 2) {
        __GXUpdateBPMask();
    }
    if (dState & 4) {
        __GXSetGenMode();
    }
    if (dState & 8) {
        __GXSetVCD();
    }
    if (dState & 0x10) {
        __GXSetVAT();
    }
    if (dState & 0x18) {
        __GXCalculateVLim();
    }

    dState &= ~0xFF;
    if (dState) {
        temp = dState & 0xF00;
        if (temp != 0) {
            __GXSetAmbMat(temp);
        }

        temp = dState & 0x100F000;
        if (temp != 0) {
            __GXSetLightChan(temp);
        }

        temp = dState & 0x2FF0000;
        if (temp != 0) {
            __GXSetTexGen(temp);
        }

        temp = dState & 0x4000000;
        if (temp != 0) {
            __GXSetMatrixIndex(GX_VA_PNMTXIDX);
            __GXSetMatrixIndex(GX_VA_TEX4MTXIDX);
        }

        temp = dState & 0x10000000;
        if (temp != 0) {
            __GXSetViewport();
        }

        temp = dState & 0x8000000;
        if (temp != 0) {
            __GXSetProjection();
        }

        __GXData->bpSentNot = 1;
    }

    __GXData->dirtyState = 0;
}
#else
void __GXSetDirtyState(void)
{
    u32 dState = __GXData->dirtyState;

    if (dState & 1) {
        __GXSetSUTexRegs();
    }
    if (dState & 2) {
        __GXUpdateBPMask();
    }
    if (dState & 4) {
        __GXSetGenMode();
    }
    if (dState & 8) {
        __GXSetVCD();
    }
    if (dState & 0x10) {
        __GXSetVAT();
    }
    if (dState & 0x18) {
        __GXCalculateVLim();
    }

    __GXData->dirtyState = 0;
}
#endif

void GXBegin(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts)
{
    ASSERTMSGLINE(0x167, vtxfmt < 8,   "GXBegin: Format Index is out of range");
    ASSERTMSGLINE(0x168, !__GXinBegin, "GXBegin: called inside another GXBegin/GXEnd");

    if (__GXData->dirtyState != 0) {
        __GXSetDirtyState();
    }
    if (*(u32 *)&__GXData->vNumNot == 0) {  // checks both vNum and bpSentNot
        __GXSendFlushPrim();
    }
    GX_WRITE_U8(vtxfmt | type);
    GX_WRITE_U16(nverts);
#endif
}

#if SDK_VERSION < SDK_VERSION_G2ME01_00
void GXSetPointSize(u8 pointSize, GXTexOffset texOffsets) {
#else
void __GXSendFlushPrim(void)
{
    u32 i;
    u32 numD = __GXData->vNum * __GXData->vLim;
#endif

#if SDK_VERSION < SDK_VERSION_G2ME01_00
  SET_REG_FIELD(__GXData->lpSize, 8, 8, pointSize);
  SET_REG_FIELD(__GXData->lpSize, 3, 19, texOffsets);
  GX_WRITE_RAS_REG(__GXData->lpSize);
  __GXData->bpSentNot = 0;
#else
    GX_WRITE_U8(0x98);
    GX_WRITE_U16(__GXData->vNum);
    for (i = 0; i < numD; i += 4) {
        GX_WRITE_U32(0);
    }
    __GXData->bpSentNot = 1;
#endif
}

#if SDK_VERSION < SDK_VERSION_G2ME01_00
void GXEnableTexOffsets(GXTexCoordID coord, u8 line_enable, u8 point_enable) {

  SET_REG_FIELD(__GXData->suTs0[coord], 1, 18, line_enable);
  SET_REG_FIELD(__GXData->suTs0[coord], 1, 19, point_enable);
  GX_WRITE_RAS_REG(__GXData->suTs0[coord]);
  __GXData->bpSentNot = 0;
#else
void GXSetLineWidth(u8 width, GXTexOffset texOffsets)
{
    CHECK_GXBEGIN(0x1B8, "GXSetLineWidth");
    SET_REG_FIELD(0x1B9, __GXData->lpSize, 8, 0, width);
    SET_REG_FIELD(0x1BA, __GXData->lpSize, 3, 16, texOffsets);
    GX_WRITE_RAS_REG(__GXData->lpSize);
    __GXData->bpSentNot = 0;
#endif
}

#if SDK_VERSION < SDK_VERSION_G2ME01_00
void GXSetCullMode(GXCullMode mode) {
  GXCullMode hwMode;
#else
void GXGetLineWidth(u8 *width, GXTexOffset *texOffsets)
{
    ASSERTMSGLINE(0x1CF, width != NULL && texOffsets != NULL, "GXGet*: invalid null pointer");
#endif

#if SDK_VERSION < SDK_VERSION_G2ME01_00
  switch (mode) {
  case GX_CULL_FRONT:
    hwMode = GX_CULL_BACK;
    break;
  case GX_CULL_BACK:
    hwMode = GX_CULL_FRONT;
    break;
  default:
    hwMode = mode;
    break;
  }

  SET_REG_FIELD(__GXData->genMode, 2, 14, hwMode);
  __GXData->dirtyState |= 4;
#else
    *width      = GET_REG_FIELD(__GXData->lpSize, 8, 0);
    *texOffsets = GET_REG_FIELD(__GXData->lpSize, 3, 16);
#endif
}

#if SDK_VERSION < SDK_VERSION_G2ME01_00
void GXSetCoPlanar(GXBool enable) {
  u32 reg;

  SET_REG_FIELD(__GXData->genMode, 1, 19, enable);
  reg = 0xFE080000;
  GX_WRITE_RAS_REG(reg);
  GX_WRITE_RAS_REG(__GXData->genMode);
#else
void GXSetPointSize(u8 pointSize, GXTexOffset texOffsets)
{
    CHECK_GXBEGIN(0x1E4, "GXSetPointSize");
    SET_REG_FIELD(0x1E5, __GXData->lpSize, 8, 8, pointSize);
    SET_REG_FIELD(0x1E6, __GXData->lpSize, 3, 19, texOffsets);
    GX_WRITE_RAS_REG(__GXData->lpSize);
    __GXData->bpSentNot = 0;
#endif
}

#if SDK_VERSION < SDK_VERSION_G2ME01_00
void __GXSetGenMode(void) {
  GX_WRITE_RAS_REG(__GXData->genMode);
  __GXData->bpSentNot = 0;
#else
void GXGetPointSize(u8 *pointSize, GXTexOffset *texOffsets)
{
    ASSERTMSGLINE(0x1FB, pointSize != NULL && texOffsets != NULL, "GXGet*: invalid null pointer");

    *pointSize  = (int)GET_REG_FIELD(__GXData->lpSize, 8, 8);
    *texOffsets = GET_REG_FIELD(__GXData->lpSize, 3, 19);
#endif
}

#if SDK_VERSION >= SDK_VERSION_G2ME01_00
void GXEnableTexOffsets(GXTexCoordID coord, u8 line_enable, u8 point_enable)
{
    CHECK_GXBEGIN(0x211, "GXEnableTexOffsets");

    ASSERTMSGLINE(0x213, coord < 8, "GXEnableTexOffsets: Invalid coordinate Id");

    SET_REG_FIELD(0x215, __GXData->suTs0[coord], 1, 18, line_enable);
    SET_REG_FIELD(0x216, __GXData->suTs0[coord], 1, 19, point_enable);
    GX_WRITE_RAS_REG(__GXData->suTs0[coord]);
    __GXData->bpSentNot = 0;
}

void GXSetCullMode(GXCullMode mode)
{
    GXCullMode hwMode;

    CHECK_GXBEGIN(0x22D, "GXSetCullMode");
#if SDK_VERSION >= SDK_VERSION_R32J01_00
    hwMode = ((mode << 1) & 2) | ((mode >> 1) & 1);
#else
    hwMode = (mode >> 1) & 1;
    __rlwimi(hwMode, mode, 1, 30, 30);
#endif

    SET_REG_FIELD(0x23A, __GXData->genMode, 2, 14, hwMode);
    __GXData->dirtyState |= 4;
}

void GXGetCullMode(GXCullMode *mode)
{
    // fake match?
    GXCullMode hwMode = __GXData->genMode;
    *mode = ((hwMode >> 0xD) & 0x2) | (((((int)hwMode >> 0xE) & 0x2) >> 0x1));
}

void GXSetCoPlanar(GXBool enable)
{
    u32 reg;

    CHECK_GXBEGIN(0x265, "GXSetCoPlanar");

    SET_REG_FIELD(0x267, __GXData->genMode, 1, 19, enable);
    reg = 0xFE080000;
    GX_WRITE_RAS_REG(reg);
    GX_WRITE_RAS_REG(__GXData->genMode);
}

void __GXSetGenMode(void)
{
    GX_WRITE_RAS_REG(__GXData->genMode);
    __GXData->bpSentNot = 0;
}

#endif