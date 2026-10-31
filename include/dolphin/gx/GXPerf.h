#ifndef _DOLPHIN_GXPERF
#define _DOLPHIN_GXPERF

#include <dolphin/types.h>
#ifdef PRIME1
#include <dolphin/gx/GXEnum.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef PRIME1

void GXSetGPMetric(GXPerf0 perf0, GXPerf1 perf1);
void GXClearGPMetric(void);
void GXReadGPMetric(u32* count0, u32* count1);
void GXSetVCacheMetric(GXVCachePerf attr);
void GXReadVCacheMetric(u32* check, u32* miss, u32* stall);
void GXClearVCacheMetric(void);
#endif

void GXReadXfRasMetric(u32* xf_wait_in, u32* xf_wait_out, u32* ras_busy, u32* clocks);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_GXPERF
