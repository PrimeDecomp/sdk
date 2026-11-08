#ifndef _THP_THPFILE_H
#define _THP_THPFILE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Size 0x30
typedef struct {
  char magic[4];
  u32 version;
  u32 bufSize;
  u32 audioMaxSamples;
  f32 frameRate;
  u32 numFrames;
  u32 firstFrameSize;
  u32 movieDataSize;
  u32 compInfoDataOffsets;
  u32 offsetDataOffsets;
  u32 movieDataOffsets;
  u32 finalFrameDataOffsets;
} THPHeader;

#ifdef __cplusplus
}
#endif

#endif
