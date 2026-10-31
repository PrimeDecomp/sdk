#ifndef METROTRK_PORTABLE_SERPOLL_H
#define METROTRK_PORTABLE_SERPOLL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "trk.h"

void TRKGetInput(void);
void TRKProcessInput(int bufferIdx);
int TRKTestForPacket(void);
DSError TRKInitializeSerialHandler(void);
DSError TRKTerminateSerialHandler(void);

extern void* gTRKInputPendingPtr;

#ifdef __cplusplus
}
#endif

#endif /* METROTRK_PORTABLE_SERPOLL_H */
