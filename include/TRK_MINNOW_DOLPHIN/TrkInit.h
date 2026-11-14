#ifndef _TRKINIT
#define _TRKINIT
#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif
void InitMetroTRK(void);
void EnableMetroTRKInterrupts(void);
extern BOOL gTRKBigEndian;
#ifdef __cplusplus
}
#endif // _TRKINIT

#endif /* #ifndef __TRKINIT_H__ */
