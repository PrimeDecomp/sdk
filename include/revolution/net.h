#ifndef REVOLUTION_NET_H
#define REVOLUTION_NET_H

#include <revolution/os.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL NETGetUniversalCalendar(OSCalendarTime* time);
const char* NETGetRexPPCVersionPrintable(void);

#ifdef __cplusplus
}
#endif

#endif // REVOLUTION_NET_H
