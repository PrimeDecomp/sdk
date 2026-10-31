#ifndef _DOLPHIN_GXMANAGE
#define _DOLPHIN_GXMANAGE

#include <dolphin/gx/GXFifo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*GXDrawSyncCallback)(u16 token);
#ifdef PRIME1

#endif
typedef void (*GXDrawDoneCallback)(void);

GXFifoObj* GXInit(void* base, u32 size);
#ifdef PRIME1
void GXAbortFrame();

void GXSetDrawSync(u16 token);
u16 GXReadDrawSync(void);
GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback cb);
#endif
GXDrawDoneCallback GXSetDrawDoneCallback(GXDrawDoneCallback cb);
#ifdef PRIME1

void GXWaitDrawDone(void);
#endif
void GXDrawDone(void);
void GXSetDrawDone(void);
void GXFlush(void);
#ifndef PRIME1
void GXAbortFrame(void);
#endif
void GXPixModeSync(void);
void GXSetMisc(GXMiscToken token, u32 val);
#ifndef PRIME1
GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback cb);
void GXSetDrawSync(u16 token);
u16 GXReadDrawSync(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_GXMANAGE
