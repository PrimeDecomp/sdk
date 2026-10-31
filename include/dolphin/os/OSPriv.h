#ifndef _DOLPHIN_OSPRIV
#define _DOLPHIN_OSPRIV

#include "dolphin/os.h"

#ifdef __cplusplus
extern "C" {
#endif

__OSExceptionHandler __OSGetExceptionHandler(__OSException exception);
#ifndef PRIME1
extern OSTime __OSStartTime;
#endif
OSTime __OSGetSystemTime();
OSTime __OSTimeToSystemTime(OSTime);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_OSPRIV
