#ifndef PF_FAT32_H
#define PF_FAT32_H
#include <revolution/types.h>
#include <revolution/vf/vf_struct.h>

s32 VFiPFFAT32_ReadFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value);
s32 VFiPFFAT32_ReadFATEntryPage(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value,
                                struct PF_CACHE_PAGE** pp_page);
s32 VFiPFFAT32_WriteFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32 value);
s32 VFiPFFAT32_WriteFATEntryPage(struct PF_VOLUME* p_vol, u32 cluster, u32 value,
                                 struct PF_CACHE_PAGE** pp_page);

#endif
