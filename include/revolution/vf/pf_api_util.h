#ifndef PF_API_UTIL_H
#define PF_API_UTIL_H
#include <revolution/types.h>
#include <revolution/vf/vf_struct.h>

s32 VFiPFAPI_ParseOpenModeString(const s8* mode_str);
s32 VFiPFAPI_convertError(s32 err);
s32 VFiPFAPI_convertReturnValue(s32 err);
void* VFiPFAPI_convertReturnValue2NULL(s32 err, void* p_stream);
s32 VFiPFAPI_convertReturnValue4unmount(s32 err);

#endif
