#ifndef _REVOLUTION_OSEXECPARAMS
#define _REVOLUTION_OSEXECPARAMS

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSExecParams {
    BOOL valid;
    u32 restartCode;
    u32 bootDol;
    void* regionStart;
    void* regionEnd;
    BOOL argsUseDefault;
    void* argsAddr;
} OSExecParams;

void __OSGetExecParams(OSExecParams* params);

#ifdef __cplusplus
}
#endif

#endif // _REVOLUTION_OSEXECPARAMS
