#ifndef PF_FAT12_H
#define PF_FAT12_H
#include <revolution/types.h>
#include <revolution/vf/vf_struct.h>

s32 VFiPFFAT12_ReadFATEntry(struct PF_VOLUME* p_vol, u16 cluster, u32* p_value);
s32 VFiPFFAT12_ReadFATEntryPage(struct PF_VOLUME* p_vol, u16 cluster, u32* p_value,
                                struct PF_CACHE_PAGE** pp_page);
s32 VFiPFFAT12_WriteFATEntry(struct PF_VOLUME* p_vol, u16 cluster, u16 value);
s32 VFiPFFAT12_WriteFATEntryPage(struct PF_VOLUME* p_vol, u16 cluster, u16 value,
                                 struct PF_CACHE_PAGE** pp_page);

#endif
