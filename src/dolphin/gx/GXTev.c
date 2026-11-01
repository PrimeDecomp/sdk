#include <SDKVersion.h>
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
#include "dolphin/gx.h"
#else
#include <dolphin/gx.h>
#include <dolphin/os.h>
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
#include "dolphin/gx/GXPriv.h"
#else
#include <dolphin/gx/__gx.h>
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
static u32 TEVCOpTableST0[] = {
    0xC008F8AF, // modulate
    0xC008A89F, // decal
    0xC008AC8F, // blend
    0xC008FFF8, // replace
    0xC008FFFA, // passclr
#else
static struct {
    u32 rid : 8;
    u32 dest : 2;
    u32 shift : 2;
    u32 clamp : 1;
    u32 sub : 1;
    u32 bias : 2;
    u32 sela : 4;
    u32 selb : 4;
    u32 selc : 4;
    u32 seld : 4;
} TEVCOpTableST0[5] = {
    {192, 0, 0, 1, 0, 0, 15, 8, 10, 15},   // modulate
    {192, 0, 0, 1, 0, 0, 10, 8, 9, 15},    // decal
    {192, 0, 0, 1, 0, 0, 10, 12, 8, 15},   // blend
    {192, 0, 0, 1, 0, 0, 15, 15, 15, 8},   // replace
    {192, 0, 0, 1, 0, 0, 15, 15, 15, 10},  // passclr
#endif
};

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
static u32 TEVCOpTableST1[] = {
    0xC008F80F, // modulate
    0xC008089F, // decal
    0xC0080C8F, // blend
    0xC008FFF8, // replace
    0xC008FFF0, // passclr
#else
static struct {
    u32 rid : 8;
    u32 dest : 2;
    u32 shift : 2;
    u32 clamp : 1;
    u32 sub : 1;
    u32 bias : 2;
    u32 sela : 4;
    u32 selb : 4;
    u32 selc : 4;
    u32 seld : 4;
} TEVCOpTableST1[5] = {
    {192, 0, 0, 1, 0, 0, 15, 8, 0, 15},   // modulate
    {192, 0, 0, 1, 0, 0, 0, 8, 9, 15},    // decal
    {192, 0, 0, 1, 0, 0, 0, 12, 8, 15},   // blend
    {192, 0, 0, 1, 0, 0, 15, 15, 15, 8},  // replace
    {192, 0, 0, 1, 0, 0, 15, 15, 15, 0},  // passclr
#endif
};

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
static u32 TEVAOpTableST0[] = {
    0xC108F2F0, // modulate
    0xC108FFD0, // decal
    0xC108F2F0, // blend
    0xC108FFC0, // replace
    0xC108FFD0, // passclr
#else
static struct {
    u32 rid : 8;
    u32 dest : 2;
    u32 shift : 2;
    u32 clamp : 1;
    u32 sub : 1;
    u32 bias : 2;
    u32 sela : 3;
    u32 selb : 3;
    u32 selc : 3;
    u32 seld : 3;
    u32 swap : 2;
    u32 mode : 2;
} TEVAOpTableST0[5] = {
    {193, 0, 0, 1, 0, 0, 7, 4, 5, 7, 0, 0},  // modulate
    {193, 0, 0, 1, 0, 0, 7, 7, 7, 5, 0, 0},  // decal
    {193, 0, 0, 1, 0, 0, 7, 4, 5, 7, 0, 0},  // blend
    {193, 0, 0, 1, 0, 0, 7, 7, 7, 4, 0, 0},  // replace
    {193, 0, 0, 1, 0, 0, 7, 7, 7, 5, 0, 0},  // passclr
#endif
};

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
static u32 TEVAOpTableST1[] = {
    0xC108F070, // modulate
    0xC108FF80, // decal
    0xC108F070, // blend
    0xC108FFC0, // replace
    0xC108FF80, // passclr
#else
static struct {
    u32 rid : 8;
    u32 dest : 2;
    u32 shift : 2;
    u32 clamp : 1;
    u32 sub : 1;
    u32 bias : 2;
    u32 sela : 3;
    u32 selb : 3;
    u32 selc : 3;
    u32 seld : 3;
    u32 swap : 2;
    u32 mode : 2;
} TEVAOpTableST1[5] = {
    {193, 0, 0, 1, 0, 0, 7, 4, 0, 7, 0, 0},  // modulate
    {193, 0, 0, 1, 0, 0, 7, 7, 7, 0, 0, 0},  // decal
    {193, 0, 0, 1, 0, 0, 7, 4, 0, 7, 0, 0},  // blend
    {193, 0, 0, 1, 0, 0, 7, 7, 7, 4, 0, 0},  // replace
    {193, 0, 0, 1, 0, 0, 7, 7, 7, 0, 0, 0},  // passclr
#endif
};

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void GXSetTevOp(GXTevStageID id, GXTevMode mode) {
  u32 *ctmp;
  u32 *atmp;
  u32 tevReg;
#else
#define SOME_SET_REG_MACRO(reg, size, shift, val)                                                   \
	do {                                                                                            \
		(reg) = (u32)__rlwimi((u32)(reg), (val), (shift), (32 - (shift) - (size)), (31 - (shift))); \
	} while (0);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (id == GX_TEVSTAGE0) {
    ctmp = (u32 *)TEVCOpTableST0 + mode;
    atmp = (u32 *)TEVAOpTableST0 + mode;
  } else {
    ctmp = (u32 *)TEVCOpTableST1 + mode;
    atmp = (u32 *)TEVAOpTableST1 + mode;
  }
#else
void GXSetTevOp(GXTevStageID id, GXTevMode mode)
{
    u32* ctmp;
    u32* atmp;
    u32 tevReg;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  tevReg = __GXData->tevc[id];
  tevReg = (*ctmp & ~0xFF000000) | (tevReg & 0xFF000000);
  GX_WRITE_RAS_REG(tevReg);
  __GXData->tevc[id] = tevReg;
#else
    CHECK_GXBEGIN(0x1A4, "GXSetTevOp");
    ASSERTMSGLINE(0x1A5, id < 16, "GXSetTevColor*: Invalid Tev Stage Index");
    ASSERTMSGLINE(0x1A6, mode <= 4, "GXSetTevOp: Invalid Tev Mode");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  tevReg = __GXData->teva[id];
  tevReg = (*atmp & ~0xFF00000F) | (tevReg & 0xFF00000F);
  GX_WRITE_RAS_REG(tevReg);
  __GXData->teva[id] = tevReg;
#else
    if (id == GX_TEVSTAGE0) {
        ctmp = (u32*)TEVCOpTableST0 + mode;
        atmp = (u32*)TEVAOpTableST0 + mode;
    } else {
        ctmp = (u32*)TEVCOpTableST1 + mode;
        atmp = (u32*)TEVAOpTableST1 + mode;
    }
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  __GXData->bpSentNot = 0;
#else
    tevReg = __GXData->tevc[id];
    tevReg = (*ctmp & ~0xFF000000) | (tevReg & 0xFF000000);
    GX_WRITE_RAS_REG(tevReg);
    __GXData->tevc[id] = tevReg;

    tevReg = __GXData->teva[id];
    tevReg = (*atmp & ~0xFF00000F) | (tevReg & 0xFF00000F);
    GX_WRITE_RAS_REG(tevReg);
    __GXData->teva[id] = tevReg;

    __GXData->bpSentNot = 0;
#endif
}

void GXSetTevColorIn(GXTevStageID stage, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c, GXTevColorArg d)
{
    u32 tevReg;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  tevReg = __GXData->tevc[stage];
  __SET_REG_FIELD(tevReg, 4, 12, a);
  __SET_REG_FIELD(tevReg, 4, 8, b);
  __SET_REG_FIELD(tevReg, 4, 4, c);
  __SET_REG_FIELD(tevReg, 4, 0, d);
#else
    CHECK_GXBEGIN(0x242, "GXSetTevColorIn");
    ASSERTMSGLINE(0x243, stage < 16, "GXSetTevColor*: Invalid Tev Stage Index");
    ASSERTMSGLINE(0x244, a <= 15, "GXSetTev*In: A/B/C/D argument out of range");
    ASSERTMSGLINE(0x245, b <= 15, "GXSetTev*In: A/B/C/D argument out of range");
    ASSERTMSGLINE(0x246, c <= 15, "GXSetTev*In: A/B/C/D argument out of range");
    ASSERTMSGLINE(0x247, d <= 15, "GXSetTev*In: A/B/C/D argument out of range");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(tevReg);
  __GXData->tevc[stage] = tevReg;
  __GXData->bpSentNot = 0;
#else
    tevReg = __GXData->tevc[stage];
    SET_REG_FIELD(0x24A, tevReg, 4, 12, a);
    SET_REG_FIELD(0x24B, tevReg, 4,  8, b);
    SET_REG_FIELD(0x24C, tevReg, 4,  4, c);
    SET_REG_FIELD(0x24D, tevReg, 4,  0, d);

    GX_WRITE_RAS_REG(tevReg);
    __GXData->tevc[stage] = tevReg;
    __GXData->bpSentNot = 0;
#endif
}

void GXSetTevAlphaIn(GXTevStageID stage, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c, GXTevAlphaArg d)
{
    u32 tevReg;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  tevReg = __GXData->teva[stage];
  __SET_REG_FIELD(tevReg, 3, 13, a);
  __SET_REG_FIELD(tevReg, 3, 10, b);
  __SET_REG_FIELD(tevReg, 3, 7, c);
  __SET_REG_FIELD(tevReg, 3, 4, d);
#else
    CHECK_GXBEGIN(0x266, "GXSetTevAlphaIn");
    ASSERTMSGLINE(0x267, stage < 16, "GXSetTevAlpha*: Invalid Tev Stage Index");
    ASSERTMSGLINE(0x268, a <= 7, "GXSetTev*In: A/B/C/D argument out of range");
    ASSERTMSGLINE(0x269, b <= 7, "GXSetTev*In: A/B/C/D argument out of range");
    ASSERTMSGLINE(0x26A, c <= 7, "GXSetTev*In: A/B/C/D argument out of range");
    ASSERTMSGLINE(0x26B, d <= 7, "GXSetTev*In: A/B/C/D argument out of range");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(tevReg);
  __GXData->teva[stage] = tevReg;
  __GXData->bpSentNot = 0;
#else
    tevReg = __GXData->teva[stage];
    SET_REG_FIELD(0x26E, tevReg, 3, 13, a);
    SET_REG_FIELD(0x26F, tevReg, 3, 10, b);
    SET_REG_FIELD(0x270, tevReg, 3,  7, c);
    SET_REG_FIELD(0x271, tevReg, 3,  4, d);

    GX_WRITE_RAS_REG(tevReg);
    __GXData->teva[stage] = tevReg;
    __GXData->bpSentNot = 0;
#endif
}

void GXSetTevColorOp(GXTevStageID stage, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp, GXTevRegID out_reg)
{
    u32 tevReg;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  tevReg = __GXData->tevc[stage];
  __SET_REG_FIELD(tevReg, 1, 18, op & 1);
  if (op <= 1) {
    __SET_REG_FIELD(tevReg, 2, 20, scale);
    __SET_REG_FIELD(tevReg, 2, 16, bias);
  } else {
    __SET_REG_FIELD(tevReg, 2, 20, (op >> 1) & 3);
    __SET_REG_FIELD(tevReg, 2, 16, 3);
  }
  __SET_REG_FIELD(tevReg, 1, 19, clamp & 0xFF);
  __SET_REG_FIELD(tevReg, 2, 22, out_reg);
#else
    CHECK_GXBEGIN(0x28D, "GXSetTevColorOp");
    ASSERTMSGLINE(0x28E, stage < 16, "GXSetTevColor*: Invalid Tev Stage Index");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(tevReg);
  __GXData->tevc[stage] = tevReg;
  __GXData->bpSentNot = 0;
#else
    tevReg = __GXData->tevc[stage];
    SET_REG_FIELD(0x297, tevReg, 1, 18, op & 1);
    if (op <= 1) {
        SET_REG_FIELD(0x299, tevReg, 2, 20, scale);
        SET_REG_FIELD(0x29A, tevReg, 2, 16, bias);
    } else {
        SET_REG_FIELD(0x29C, tevReg, 2, 20, (op >> 1) & 3);
        SET_REG_FIELD(0x2A0, tevReg, 2, 16, 3);
    }
    SET_REG_FIELD(0x2A0, tevReg, 1, 19, clamp & 0xFF);
    SET_REG_FIELD(0x2A1, tevReg, 2, 22, out_reg);

    GX_WRITE_RAS_REG(tevReg);
    __GXData->tevc[stage] = tevReg;
    __GXData->bpSentNot = 0;
#endif
}

void GXSetTevAlphaOp(GXTevStageID stage, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp, GXTevRegID out_reg)
{
    u32 tevReg;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  tevReg = __GXData->teva[stage];
  __SET_REG_FIELD(tevReg, 1, 18, op & 1);
  if (op <= 1) {
    __SET_REG_FIELD(tevReg, 2, 20, scale);
    __SET_REG_FIELD(tevReg, 2, 16, bias);
  } else {
    __SET_REG_FIELD(tevReg, 2, 20, (op >> 1) & 3);
    __SET_REG_FIELD(tevReg, 2, 16, 3);
  }
  __SET_REG_FIELD(tevReg, 1, 19, clamp & 0xFF);
  __SET_REG_FIELD(tevReg, 2, 22, out_reg);
#else
    CHECK_GXBEGIN(0x2BB, "GXSetTevAlphaOp");
    ASSERTMSGLINE(0x2BC, stage < 16, "GXSetTevAlpha*: Invalid Tev Stage Index");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(tevReg);
  __GXData->teva[stage] = tevReg;
  __GXData->bpSentNot = 0;
#else
    tevReg = __GXData->teva[stage];
    SET_REG_FIELD(0x2C4, tevReg, 1, 18, op & 1);
    if (op <= 1) {
        SET_REG_FIELD(0x2C6, tevReg, 2, 20, scale);
        SET_REG_FIELD(0x2C7, tevReg, 2, 16, bias);
    } else {
        SET_REG_FIELD(0x2C9, tevReg, 2, 20, (op >> 1) & 3);
        SET_REG_FIELD(0x2CD, tevReg, 2, 16, 3);
    }
    SET_REG_FIELD(0x2CD, tevReg, 1, 19, clamp & 0xFF);
    SET_REG_FIELD(0x2CE, tevReg, 2, 22, out_reg);

    GX_WRITE_RAS_REG(tevReg);
    __GXData->teva[stage] = tevReg;
    __GXData->bpSentNot = 0;
#endif
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void GXSetTevColor(GXTevRegID id, GXColor color) {
  u32 regRA;
  u32 regBG;
#else
void GXSetTevColor(GXTevRegID id, GXColor color)
{
    u32 rgba;
    u32 regRA;
    u32 regBG;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  regRA = 0;
  SET_REG_FIELD(regRA, 11, 0, color.r);
  SET_REG_FIELD(regRA, 11, 12, color.a);
  SET_REG_FIELD(regRA, 8, 24, 224 + id * 2);
#else
    CHECK_GXBEGIN(0x2E4, "GXSetTevColor");
    rgba = *(u32*)&color;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  regBG = 0;
  SET_REG_FIELD(regBG, 11, 0, color.b);
  SET_REG_FIELD(regBG, 11, 12, color.g);
  SET_REG_FIELD(regBG, 8, 24, 225 + id * 2);
#else
    regRA = (0xE0 + id * 2) << 24;
    SET_REG_FIELD(0x2E9, regRA, 8, 0, rgba >> 24);
    SET_REG_FIELD(0x2EA, regRA, 8, 12, rgba & 0xFF);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(regRA);
  GX_WRITE_RAS_REG(regBG);
  GX_WRITE_RAS_REG(regBG);
  GX_WRITE_RAS_REG(regBG);
#else
    regBG = (0xE1 + id * 2) << 24;
    SET_REG_FIELD(0x2ED, regBG, 8, 0, (rgba >> 8) & 0xFF);
    SET_REG_FIELD(0x2EE, regBG, 8, 12, (rgba >> 16) & 0xFF);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  __GXData->bpSentNot = 0;
#else
    GX_WRITE_RAS_REG(regRA);
    GX_WRITE_RAS_REG(regBG);
    GX_WRITE_RAS_REG(regBG);
    GX_WRITE_RAS_REG(regBG);

    __GXData->bpSentNot = 0;
#endif
}

void GXSetTevColorS10(GXTevRegID id, GXColorS10 color)
{
    u32 sRG;
    u32 sBA;
    u32 regRA;
    u32 regBG;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  regRA = 0;
  SET_REG_FIELD(regRA, 11, 0, color.r & 0x7FF);
  SET_REG_FIELD(regRA, 11, 12, color.a & 0x7FF);
  SET_REG_FIELD(regRA, 8, 24, 224 + id * 2);
#else
    ASSERTMSGLINE(0x309, color.r >= -1024 && color.r < 1024, "GXSetTevColorS10: Color not in range -1024 to +1023");
    ASSERTMSGLINE(0x30A, color.g >= -1024 && color.g < 1024, "GXSetTevColorS10: Color not in range -1024 to +1023");
    ASSERTMSGLINE(0x30B, color.b >= -1024 && color.b < 1024, "GXSetTevColorS10: Color not in range -1024 to +1023");
    ASSERTMSGLINE(0x30C, color.a >= -1024 && color.a < 1024, "GXSetTevColorS10: Color not in range -1024 to +1023");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  regBG = 0;
  SET_REG_FIELD(regBG, 11, 0, color.b & 0x7FF);
  SET_REG_FIELD(regBG, 11, 12, color.g & 0x7FF);
  SET_REG_FIELD(regBG, 8, 24, 225 + id * 2);
#else
    CHECK_GXBEGIN(0x30E, "GXSetTevColorS10");
    sRG = *(u32*)&color;
    sBA = *((u32*)&color + 1);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(regRA);
  GX_WRITE_RAS_REG(regBG);
  GX_WRITE_RAS_REG(regBG);
  GX_WRITE_RAS_REG(regBG);
#else
    regRA = (0xE0 + id * 2) << 24;
    SET_REG_FIELD(0x315, regRA, 11,  0, (sRG >> 16) & 0x7FF);
    SET_REG_FIELD(0x316, regRA, 11, 12, sBA & 0x7FF);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  __GXData->bpSentNot = 0;
#else
    regBG = (0xE1 + id * 2) << 24;
    SET_REG_FIELD(0x319, regBG, 11, 0, (sBA >> 16) & 0x7FF);
    SET_REG_FIELD(0x31A, regBG, 11, 12, sRG & 0x7FF);

    GX_WRITE_RAS_REG(regRA);
    GX_WRITE_RAS_REG(regBG);
    GX_WRITE_RAS_REG(regBG);
    GX_WRITE_RAS_REG(regBG);

    __GXData->bpSentNot = 0;
#endif
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void GXSetTevKColor(GXTevKColorID id, GXColor color) {
  u32 regRA;
  u32 regBG;
#else
void GXSetTevKColor(GXTevKColorID id, GXColor color)
{
    u32 rgba;
    u32 regRA;
    u32 regBG;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  regRA = 0;
  SET_REG_FIELD(regRA, 8, 0, color.r);
  SET_REG_FIELD(regRA, 8, 12, color.a);
  SET_REG_FIELD(regRA, 4, 20, 8);
  SET_REG_FIELD(regRA, 8, 24, 224 + id * 2);
#else
    CHECK_GXBEGIN(0x341, "GXSetTevKColor");
    rgba = *(u32*)&color;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  regBG = 0;
  SET_REG_FIELD(regBG, 8, 0, color.b);
  SET_REG_FIELD(regBG, 8, 12, color.g);
  SET_REG_FIELD(regBG, 4, 20, 8);
  SET_REG_FIELD(regBG, 8, 24, 225 + id * 2);
#else
    regRA = (0xE0 + id * 2) << 24;
    SET_REG_FIELD(0x346, regRA, 8, 0, rgba >> 24);
    SET_REG_FIELD(0x347, regRA, 8, 12, rgba & 0xFF);
    SET_REG_FIELD(0x347, regRA, 4, 20, 8);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(regRA);
  GX_WRITE_RAS_REG(regBG);
  __GXData->bpSentNot = 0;
#else
    regBG = (0xE1 + id * 2) << 24;
    SET_REG_FIELD(0x34B, regBG, 8, 0, (rgba >> 8) & 0xFF);
    SET_REG_FIELD(0x34C, regBG, 8, 12, (rgba >> 16) & 0xFF);
    SET_REG_FIELD(0x34D, regBG, 4, 20, 8);

    GX_WRITE_RAS_REG(regRA);
    GX_WRITE_RAS_REG(regBG);

    __GXData->bpSentNot = 0;
#endif
}

void GXSetTevKColorSel(GXTevStageID stage, GXTevKColorSel sel)
{
    u32 *Kreg;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  Kreg = &__GXData->tevKsel[stage >> 1];
  if (stage & 1) {
    SET_REG_FIELD(*Kreg, 5, 14, sel);
  } else {
    SET_REG_FIELD(*Kreg, 5, 4, sel);
  }
#else
    CHECK_GXBEGIN(0x368, "GXSetTevKColorSel");
    ASSERTMSGLINE(0x369, stage < 16, "GXSetTevKColor*: Invalid Tev Stage Index");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(*Kreg);
  __GXData->bpSentNot = 0;
#else
    Kreg = &__GXData->tevKsel[stage >> 1];
    if (stage & 1) {
        SET_REG_FIELD(0x36E, *Kreg, 5, 14, sel);
    } else {
        SET_REG_FIELD(0x370, *Kreg, 5, 4, sel);
    }

    GX_WRITE_RAS_REG(*Kreg);
    __GXData->bpSentNot = 0;
#endif
}

void GXSetTevKAlphaSel(GXTevStageID stage, GXTevKAlphaSel sel)
{
    u32 *Kreg;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  Kreg = &__GXData->tevKsel[stage >> 1];
  if (stage & 1) {
    SET_REG_FIELD(*Kreg, 5, 19, sel);
  } else {
    SET_REG_FIELD(*Kreg, 5, 9, sel);
  }
#else
    CHECK_GXBEGIN(0x389, "GXSetTevKAlphaSel");
    ASSERTMSGLINE(0x38A, stage < 16, "GXSetTevKColor*: Invalid Tev Stage Index");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(*Kreg);
  __GXData->bpSentNot = 0;
#else
    Kreg = &__GXData->tevKsel[stage >> 1];
    if (stage & 1) {
        SET_REG_FIELD(0x38F, *Kreg, 5, 19, sel);
    } else {
        SET_REG_FIELD(0x391, *Kreg, 5, 9, sel);
    }

    GX_WRITE_RAS_REG(*Kreg);
    __GXData->bpSentNot = 0;
#endif
}

void GXSetTevSwapMode(GXTevStageID stage, GXTevSwapSel ras_sel, GXTevSwapSel tex_sel)
{
    u32 *pTevReg;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  pTevReg = &__GXData->teva[stage];
  SET_REG_FIELD(*pTevReg, 2, 0, ras_sel);
  SET_REG_FIELD(*pTevReg, 2, 2, tex_sel);
#else
    CHECK_GXBEGIN(0x3AE, "GXSetTevSwapMode");
    ASSERTMSGLINE(0x3AF, stage < 16, "GXSetTevSwapMode: Invalid Tev Stage Index");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(*pTevReg);
  __GXData->bpSentNot = 0;
#else
    pTevReg = &__GXData->teva[stage];
    SET_REG_FIELD(0x3B2, *pTevReg, 2, 0, ras_sel);
    SET_REG_FIELD(0x3B3, *pTevReg, 2, 2, tex_sel);

    GX_WRITE_RAS_REG(*pTevReg);
    __GXData->bpSentNot = 0;
#endif
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void GXSetTevSwapModeTable(GXTevSwapSel table, GXTevColorChan red, GXTevColorChan green,
                           GXTevColorChan blue, GXTevColorChan alpha) {
  u32 *Kreg;
#else
void GXSetTevSwapModeTable(GXTevSwapSel table, GXTevColorChan red, GXTevColorChan green, GXTevColorChan blue, GXTevColorChan alpha)
{
    u32 *Kreg;
    // not a real variable, but needed to match release
    int index = table * 2;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  int index = table * 2;
#else
    CHECK_GXBEGIN(0x3D2, "GXSetTevSwapModeTable");
    ASSERTMSGLINE(0x3D3, table < 4, "GXSetTevSwapModeTable: Invalid Swap Selection Index");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  Kreg = &__GXData->tevKsel[index];
#else
    Kreg = &__GXData->tevKsel[index];
    SET_REG_FIELD(0x3D6, *Kreg, 2, 0, red);
    SET_REG_FIELD(0x3D7, *Kreg, 2, 2, green);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  SET_REG_FIELD(*Kreg, 2, 0, red);
  SET_REG_FIELD(*Kreg, 2, 2, green);
#else
    GX_WRITE_RAS_REG(*Kreg);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  GX_WRITE_RAS_REG(*Kreg);
#else
    Kreg = &__GXData->tevKsel[table * 2 + 1];
    SET_REG_FIELD(0x3DB, *Kreg, 2, 0, blue);
    SET_REG_FIELD(0x3DC, *Kreg, 2, 2, alpha);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  Kreg = &__GXData->tevKsel[table * 2 + 1];
  SET_REG_FIELD(*Kreg, 2, 0, blue);
  SET_REG_FIELD(*Kreg, 2, 2, alpha);

  GX_WRITE_RAS_REG(*Kreg);
  __GXData->bpSentNot = 0;
#else
    GX_WRITE_RAS_REG(*Kreg);
    __GXData->bpSentNot = 0;
#endif
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void GXSetAlphaCompare(GXCompare comp0, u8 ref0, GXAlphaOp op, GXCompare comp1, u8 ref1) {
  u32 reg;

  reg = 0xF3000000;

  SET_REG_FIELD(reg, 8, 0, ref0);
  SET_REG_FIELD(reg, 8, 8, ref1);
  SET_REG_FIELD(reg, 3, 16, comp0);
  SET_REG_FIELD(reg, 3, 19, comp1);
  SET_REG_FIELD(reg, 2, 22, op);

  GX_WRITE_RAS_REG(reg);
  __GXData->bpSentNot = 0;
#else
void GXSetTevClampMode(void)
{
    ASSERTMSGLINE(0x3F4, 0, "GXSetTevClampMode: not available on this hardware");
#endif
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void GXSetZTexture(GXZTexOp op, GXTexFmt fmt, u32 bias) {
  u32 zenv0;
  u32 zenv1;
  u32 type;
#else
void GXSetAlphaCompare(GXCompare comp0, u8 ref0, GXAlphaOp op, GXCompare comp1, u8 ref1)
{
    u32 reg;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  zenv0 = 0;
  SET_REG_FIELD(zenv0, 24, 0, bias);
  SET_REG_FIELD(zenv0, 8, 24, 0xF4);
#else
    CHECK_GXBEGIN(0x416, "GXSetAlphaCompare");
    reg = 0xF3000000;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  zenv1 = 0;
  switch (fmt) {
  case GX_TF_Z8:
    type = 0;
    break;
  case GX_TF_Z16:
    type = 1;
    break;
  case GX_TF_Z24X8:
    type = 2;
    break;
  default:
#else
    SET_REG_FIELD(0x419, reg, 8, 0, ref0);
    SET_REG_FIELD(0x41A, reg, 8, 8, ref1);
    SET_REG_FIELD(0x41B, reg, 3, 16, comp0);
    SET_REG_FIELD(0x41C, reg, 3, 19, comp1);
    SET_REG_FIELD(0x41D, reg, 2, 22, op);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    type = 2;
    break;
  }

  SET_REG_FIELD(zenv1, 2, 0, type);
  SET_REG_FIELD(zenv1, 2, 2, op);
  SET_REG_FIELD(zenv1, 8, 24, 0xF5);

  GX_WRITE_RAS_REG(zenv0);
  GX_WRITE_RAS_REG(zenv1);
  __GXData->bpSentNot = 0;
#else
    GX_WRITE_RAS_REG(reg);
    __GXData->bpSentNot = 0;
#endif
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void GXSetTevOrder(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, GXChannelID color) {
  u32 *ptref;
  u32 tmap;
  u32 tcoord;
  static int c2r[] = {0, 1, 0, 1, 0, 1, 7, 5, 6};
#else
void GXSetZTexture(GXZTexOp op, GXTexFmt fmt, u32 bias)
{
    u32 zenv0;
    u32 zenv1;
    u32 type;
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  ptref = &__GXData->tref[stage / 2];
  __GXData->texmapId[stage] = map;
#else
    CHECK_GXBEGIN(0x435, "GXSetZTexture");
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  tmap = map & ~GX_TEX_DISABLE;
  tmap = (tmap >= GX_MAX_TEXMAP) ? GX_TEXMAP0 : tmap;
#else
    zenv0 = 0;
    SET_REG_FIELD(0x438, zenv0, 24, 0, bias);
    SET_REG_FIELD(0x439, zenv0, 8, 24, 0xF4);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (coord >= GX_MAX_TEXCOORD) {
    tcoord = GX_TEXCOORD0;
    __GXData->tevTcEnab = __GXData->tevTcEnab & ~(1 << stage);
  } else {
    tcoord = coord;
    __GXData->tevTcEnab = __GXData->tevTcEnab | (1 << stage);
  }
#else
    zenv1 = 0;
    switch (fmt)
    {
    case GX_TF_Z8:
        type = 0;
        break;
    case GX_TF_Z16:
        type = 1;
        break;
    case GX_TF_Z24X8:
        type = 2;
        break;
    default:
        ASSERTMSGLINE(0x441, 0, "GXSetZTexture: Invalid z-texture format");
        type = 2;
        break;
    }
    SET_REG_FIELD(0x444, zenv1, 2, 0, type);
    SET_REG_FIELD(0x445, zenv1, 2, 2, op);
    SET_REG_FIELD(0x446, zenv1, 8, 24, 0xF5);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (stage & 1) {
    SET_REG_FIELD(*ptref, 3, 12, tmap);
    SET_REG_FIELD(*ptref, 3, 15, tcoord);
    SET_REG_FIELD(*ptref, 3, 19, (color == GX_COLOR_NULL) ? 7 : c2r[color]);
    SET_REG_FIELD(*ptref, 1, 18, (map != GX_TEXMAP_NULL && !(map & GX_TEX_DISABLE)));
  } else {
    SET_REG_FIELD(*ptref, 3, 0, tmap);
    SET_REG_FIELD(*ptref, 3, 3, tcoord);
    SET_REG_FIELD(*ptref, 3, 7, (color == GX_COLOR_NULL) ? 7 : c2r[color]);
    SET_REG_FIELD(*ptref, 1, 6, (map != GX_TEXMAP_NULL && !(map & GX_TEX_DISABLE)));
  }

  GX_WRITE_RAS_REG(*ptref);
  __GXData->bpSentNot = 0;
  __GXData->dirtyState |= 1;
#else
    GX_WRITE_RAS_REG(zenv0);
    GX_WRITE_RAS_REG(zenv1);
    __GXData->bpSentNot = 0;
#endif
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void GXSetNumTevStages(u8 nStages) {
#else
void GXSetTevOrder(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, GXChannelID color)
{
    u32 *ptref;
    u32 tmap;
    u32 tcoord;
    static int c2r[] = { 0, 1, 0, 1, 0, 1, 7, 5, 6 };
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  SET_REG_FIELD(__GXData->genMode, 4, 10, nStages - 1);
  __GXData->dirtyState |= 4;
#else
    CHECK_GXBEGIN(0x46B, "GXSetTevOrder");
    ASSERTMSGLINE(0x46C, stage < 16, "GXSetTevOrder: Invalid Tev Stage Index");
    ASSERTMSGLINE(0x46E, coord < 8 || coord == 0xFF, "GXSetTevOrder: Invalid Texcoord");
    ASSERTMSGLINE(0x470, (map & ~0x100) < 8 || map == 0xFF, "GXSetTevOrder: Invalid Tex Map");
    ASSERTMSGLINE(0x472, color >= 4 && color <= 0xFF, "GXSetTevOrder: Invalid Color Channel ID");

    ptref = &__GXData->tref[stage / 2];
    __GXData->texmapId[stage] = map;

    tmap = map & ~0x100;
    tmap = (tmap >= GX_MAX_TEXMAP) ? GX_TEXMAP0 : tmap;

    if (coord >= GX_MAX_TEXCOORD) {
        tcoord = GX_TEXCOORD0;
        __GXData->tevTcEnab = __GXData->tevTcEnab & ~(1 << stage);
    } else {
        tcoord = coord;
        __GXData->tevTcEnab = __GXData->tevTcEnab | (1 << stage);
    }

    if (stage & 1) {
        SET_REG_FIELD(0x486, *ptref, 3, 12, tmap);
        SET_REG_FIELD(0x487, *ptref, 3, 15, tcoord);
        SET_REG_FIELD(0x489, *ptref, 3, 19, (color == GX_COLOR_NULL) ? 7 : c2r[color]);
        SET_REG_FIELD(0x48B, *ptref, 1, 18, (map != GX_TEXMAP_NULL && !(map & 0x100)));
    } else {
        SET_REG_FIELD(0x48E, *ptref, 3, 0, tmap);
        SET_REG_FIELD(0x48F, *ptref, 3, 3, tcoord);
        SET_REG_FIELD(0x491, *ptref, 3, 7, (color == GX_COLOR_NULL) ? 7 : c2r[color]);
        SET_REG_FIELD(0x493, *ptref, 1, 6, (map != GX_TEXMAP_NULL && !(map & 0x100)));
    }

    GX_WRITE_RAS_REG(*ptref);
    __GXData->bpSentNot = 0;
    __GXData->dirtyState |= 1;
#endif
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04_OS_APR
void GXSetNumTevStages(u8 nStages)
{
    CHECK_GXBEGIN(0x4A3, "GXSetNumTevStages");

    ASSERTMSGLINE(0x4A5, nStages != 0 && nStages <= 16, "GXSetNumTevStages: Exceed max number of tex stages");
    SET_REG_FIELD(0x4A6, __GXData->genMode, 4, 10, nStages - 1);
    __GXData->dirtyState |= 4;
}

#endif
