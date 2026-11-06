#include "dolphin/os.h"
#include "TRK_MINNOW_DOLPHIN/MetroTRK/Portable/circle_buffer.h"

void MWExitCriticalSection(BOOL* cs) { OSRestoreInterrupts(*cs); }

void MWEnterCriticalSection(BOOL* cs) { *cs = OSDisableInterrupts(); }

void MWInitializeCriticalSection(BOOL* cs) { }
