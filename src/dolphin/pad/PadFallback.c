#include <dolphin/pad.h>

// PADRecalibrate compatibility fallback; the reset path in main.cpp still calls it as Echoes
// does, but it does nothing here.
BOOL PADRecalibrate(u32 mask) { return FALSE; }

// __PADDisableRecalibration compatibility fallback; returns false.
BOOL __PADDisableRecalibration(BOOL disable) { return FALSE; }
