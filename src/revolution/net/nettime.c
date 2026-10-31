#include <dolphin/os/OSPriv.h>
#include <revolution/net.h>
#include <revolution/nwc24.h>
#include <revolution/nwc24/NWC24DateParser.h>
#include <revolution/nwc24/NWC24Time.h>
#include <revolution/os.h>

BOOL NETGetUniversalCalendar(OSCalendarTime *pTime) {
  static s64 whenCached = 0;
  NWC24iDate date;
  s64 universalTime;

  if (whenCached == 0 || whenCached + OS_TIMER_CLOCK * 60 < __OSGetSystemTime()) {
    NWC24iSynchronizeRtcCounter(FALSE);
    whenCached = __OSGetSystemTime();
  }

  if (NWC24iGetUniversalTime(&universalTime) >= 0 &&
      NWC24iEpochSecondsToDate(&date, universalTime) >= 0 &&
      NWC24iDateToOSCalendarTime(pTime, &date) >= 0) {
    return TRUE;
  }

  OSTicksToCalendarTime(OSGetTime(), pTime);
  return FALSE;
}
