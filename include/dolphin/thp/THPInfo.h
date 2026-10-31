#ifndef _THP_THPINFO_H
#define _THP_THPINFO_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Size 0xC
typedef struct THPVideoInfo {
#ifdef PRIME1
	u32 mXSize;     // _00
	u32 mYSize;     // _04
	u32 mVideoType; // _08
#else
  u32 mXSize;     // _00
  u32 mYSize;     // _04
  u32 mVideoType; // _08
#endif
} THPVideoInfo;

typedef struct THPVideoInfoOld {
#ifdef PRIME1
  u32 mXSize;     // _00
  u32 mYSize;     // _04
#else
  u32 mXSize; // _00
  u32 mYSize; // _04
#endif
} THPVideoInfoOld;

// Size 0x10
typedef struct THPAudioInfo {
#ifdef PRIME1
	u32 mSndChannels;   // _00
	u32 mSndFrequency;  // _04
	u32 mSndNumSamples; // _08
	u32 mSndNumTracks;  // _0C
#else
  u32 mSndChannels;   // _00
  u32 mSndFrequency;  // _04
  u32 mSndNumSamples; // _08
  u32 mSndNumTracks;  // _0C
#endif
} THPAudioInfo;

// Size 0xC
typedef struct THPAudioInfoOld {
  u32 mSndChannels;   // _00
  u32 mSndFrequency;  // _04
  u32 mSndNumSamples; // _08
} THPAudioInfoOld;

// Size 0x14
typedef struct THPFrameCompInfo {
#ifdef PRIME1
	u32 mNumComponents; // _00
	u8 mFrameComp[16];  // _04
#else
  u32 mNumComponents; // _00
  u8 mFrameComp[16];  // _04
#endif
} THPFrameCompInfo;

#ifdef __cplusplus
}
#endif

#endif
