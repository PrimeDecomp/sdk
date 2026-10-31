#ifndef PF_CLUSTER_H
#define PF_CLUSTER_H
#include <revolution/types.h>
#include <revolution/vf/vf_struct.h>

void VFiPFCLUSTER_UpdateLastAccessCluster(struct PF_FILE* p_file, u32 sector);
s32 VFiPFCLUSTER_AppendCluster(struct PF_FILE* p_file, u32 byte, u32* p_success, u32* sector);
s32 VFiPFCLUSTER_GetAppendSize(struct PF_FILE* p_file, u32* p_size);

#endif
