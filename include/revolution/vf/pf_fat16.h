#ifndef PF_FAT16_H
#define PF_FAT16_H
#include <revolution/types.h>
#include <revolution/vf/vf_struct.h>

s32 VFiPFFAT16_ReadFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value);
s32 VFiPFFAT16_ReadFATEntryPage(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value,
                                struct PF_CACHE_PAGE** pp_page);
s32 VFiPFFAT16_WriteFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32 value);
s32 VFiPFFAT16_WriteFATEntryPage(struct PF_VOLUME* p_vol, u32 cluster, u32 value,
                                 struct PF_CACHE_PAGE** pp_page);

#endif
