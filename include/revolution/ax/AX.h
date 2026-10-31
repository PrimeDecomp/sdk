#ifndef RVL_SDK_AX_H
#define RVL_SDK_AX_H
#include <dolphin/types.h>
#ifdef __cplusplus
extern "C" {
#endif

void AXInit(void);
void AXInitEx(u32 mode);
BOOL AXIsInit(void);

#ifdef __cplusplus
}
#endif
#endif
