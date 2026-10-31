#ifndef _REVOLUTION_OSIPC
#define _REVOLUTION_OSIPC

#ifdef __cplusplus
extern "C" {
#endif

void* __OSGetIPCBufferHi(void);
void* __OSGetIPCBufferLo(void);
void __OSInitIPCBuffer(void);

#ifdef __cplusplus
}
#endif

#endif // _REVOLUTION_OSIPC
