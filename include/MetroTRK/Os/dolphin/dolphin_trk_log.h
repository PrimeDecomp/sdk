#ifndef OS_DOLPHIN_DOLPHIN_TRK_LOG_H
#define OS_DOLPHIN_DOLPHIN_TRK_LOG_H

#include "trk.h"

#ifdef __cplusplus
extern "C" {
#endif

// The message layer logs through an unprototyped OSReport and writes frames
// to the UART without pulling in the OS headers.
void OSReport();
int TRKWriteUARTN(const void* src, u32 n);

#ifdef __cplusplus
}
#endif

#endif /* OS_DOLPHIN_DOLPHIN_TRK_LOG_H */
