#ifndef METROTRK_PORTABLE_NUBINIT_H
#define METROTRK_PORTABLE_NUBINIT_H

#include "trk.h"

#ifdef __cplusplus
extern "C" {
#endif

void TRKNubWelcome(void);
void TRKNubMainLoop(void);
DSError TRKTerminateNub(void);
DSError TRKInitializeNub(void);
BOOL TRKInitializeEndian(void);

extern BOOL gTRKBigEndian;

#ifdef __cplusplus
}
#endif

#endif /* METROTRK_PORTABLE_NUBINIT_H */
