#ifndef _DOLPHIN_OSALARM
#define _DOLPHIN_OSALARM

#ifndef PRIME1
#include <GameVersions.h>
#endif
#include <dolphin/os/OSContext.h>
#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSAlarm OSAlarm;
typedef void (*OSAlarmHandler)(OSAlarm* alarm, OSContext* context);

struct OSAlarm {
  OSAlarmHandler handler;
  u32 tag;
  OSTime fire;
  OSAlarm* prev;
  OSAlarm* next;
  OSTime period;
  OSTime start;
#ifndef PRIME1
#if VERSION >= VERSION_R32J01
  void* userData;
#endif
#endif
};

void OSInitAlarm(void);
void OSSetAlarm(OSAlarm* alarm, OSTime tick, OSAlarmHandler handler);
void OSSetAlarmTag(OSAlarm* alarm, u32 tag);
void OSSetAbsAlarm(OSAlarm* alarm, OSTime time, OSAlarmHandler handler);
void OSSetPeriodicAlarm(OSAlarm* alarm, OSTime start, OSTime period, OSAlarmHandler handler);
void OSCreateAlarm(OSAlarm* alarm);
void OSCancelAlarm(OSAlarm* alarm);
void OSCancelAlarms(u32 tag);
#ifndef PRIME1
#if VERSION >= VERSION_R32J01
void OSSetAlarmUserData(OSAlarm* alarm, void* userData);
void* OSGetAlarmUserData(const OSAlarm* alarm);
#endif
#endif

BOOL OSCheckAlarmQueue(void);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_OSALARM
