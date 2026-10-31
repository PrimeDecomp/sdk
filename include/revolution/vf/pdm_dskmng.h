#ifndef PDM_DSKMNG_H
#define PDM_DSKMNG_H
#include <revolution/types.h>
#include <revolution/vf/vf_struct.h>

s32 VFipdm_init_diskmanager(u32 config, void* param);
s32 VFipdm_open_disk(struct PDM_INIT_DISK* p_init_disk_tbl, struct PDM_DISK** pp_disk);
s32 VFipdm_close_disk(struct PDM_DISK* p_disk);
s32 VFipdm_open_partition(struct PDM_DISK* p_disk, s32 part_id, struct PDM_PARTITION** pp_part);
s32 VFipdm_close_partition(struct PDM_PARTITION* p_part);

#endif
