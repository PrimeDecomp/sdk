#ifndef _REVOLUTION_OS
#define _REVOLUTION_OS

#ifdef PRIME1
#include <dolphin/types.h>
#else
#include <dolphin/os.h>
#endif

#ifdef PRIME1
#ifdef __cplusplus
extern "C" {
#endif
#else
#include <revolution/os/OSIpc.h>
#include <revolution/os/OSNandbootinfo.h>
#include <revolution/os/OSPlayRecord.h>
#include <revolution/os/OSPlayTime.h>
#include <revolution/os/OSStateFlags.h>
#include <revolution/os/OSStateTM.h>
#include <revolution/os/OSTime.h>
#include <revolution/os/OSUtf.h>
#endif

#ifdef PRIME1
BOOL OSDisableInterrupts(void);
BOOL OSEnableInterrupts(void);
BOOL OSRestoreInterrupts(BOOL level);

#ifdef __cplusplus
}
#endif
#else
// Marks the line of an assertion that the release build compiled away.
#define OSAssert_Line(line_, exp_)
#endif

#endif // _REVOLUTION_OS
