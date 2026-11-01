#include <SDKVersion.h>
#include "dolphin/mtx.h"
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04_OS_APR
#include "dolphin/os.h"

void C_MTXMultVec(const Mtx m, const Vec *src, Vec *dst) {
  Vec vTmp;

#line 66
  ASSERTMSG(m, "MTXMultVec():  NULL MtxPtr 'm' ");
  ASSERTMSG(src, "MTXMultVec():  NULL VecPtr 'src' ");
  ASSERTMSG(dst, "MTXMultVec():  NULL VecPtr 'dst' ");

  vTmp.x = m[0][3] + ((m[0][2] * src->z) + ((m[0][0] * src->x) + (m[0][1] * src->y)));
  vTmp.y = m[1][3] + ((m[1][2] * src->z) + ((m[1][0] * src->x) + (m[1][1] * src->y)));
  vTmp.z = m[2][3] + ((m[2][2] * src->z) + ((m[2][0] * src->x) + (m[2][1] * src->y)));
  dst->x = vTmp.x;
  dst->y = vTmp.y;
  dst->z = vTmp.z;
}
#endif

asm void PSMTXMultVec(const register Mtx m, const register Vec *in, register Vec *out) {
  // clang-format off
  nofralloc;
  psq_l fp0, 0(in), 0, 0;
  psq_l fp2, 0(m), 0, 0;
  psq_l fp1, 8(in), 1, 0;
  ps_mul fp4, fp2, fp0;
  psq_l fp3, 8(m), 0, 0;
  ps_madd fp5, fp3, fp1, fp4;
  psq_l fp8, 16(m), 0, 0;
  ps_sum0 fp6, fp5, fp6, fp5;
  psq_l fp9, 24(m), 0, 0;
  ps_mul fp10, fp8, fp0;
  psq_st fp6, 0(out), 1, 0;
  ps_madd fp11, fp9, fp1, fp10;
  psq_l fp2, 32(m), 0, 0;
  ps_sum0 fp12, fp11, fp12, fp11;
  psq_l fp3, 40(m), 0, 0;
  ps_mul fp4, fp2, fp0;
  psq_st fp12, 4(out), 1, 0;
  ps_madd fp5, fp3, fp1, fp4;
  ps_sum0 fp6, fp5, fp6, fp5;
  psq_st fp6, 8(out), 1, 0;
  blr
  // clang-format on
}

asm void PSMTXMultVecSR(const register Mtx mtx, const register Vec *in, register Vec *out) {
  // clang-format off
  nofralloc;
  psq_l fp0, 0(mtx), 0, 0;
  psq_l fp6, 0(in), 0, 0;
  psq_l fp2, 0x10(mtx), 0, 0;
  ps_mul fp8, fp0, fp6;
  psq_l fp4, 0x20(mtx), 0, 0;
  ps_mul fp10, fp2, fp6;
  psq_l fp7, 8(in), 1, 0;
  ps_mul fp12, fp4, fp6;
  psq_l fp3, 0x18(mtx), 0, 0;
  ps_sum0 fp8, fp8, fp8, fp8;
  psq_l fp5, 0x28(mtx), 0, 0;
  ps_sum0 fp10, fp10, fp10, fp10;
  psq_l fp1, 8(mtx), 0, 0;
  ps_sum0 fp12, fp12, fp12, fp12;
  ps_madd fp9, fp1, fp7, fp8;
  psq_st fp9, 0(out), 1, 0;
  ps_madd fp11, fp3, fp7, fp10;
  psq_st fp11, 4(out), 1, 0;
  ps_madd fp13, fp5, fp7, fp12;
  psq_st fp13, 8(out), 1, 0;
  blr
  // clang-format on
}
