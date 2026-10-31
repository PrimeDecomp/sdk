#include <revolution/os.h>
#include <revolution/vf/d_time.h>

void dTM_GetNowTime(PF_SYS_DATE *sdate, PF_SYS_TIME *stime) {
  OSTime now;
  OSCalendarTime calendarTime;

  sdate->sys_year = 0;
  sdate->sys_month = 0;
  sdate->sys_day = 0;
  stime->sys_hour = 0;
  stime->sys_min = 0;
  stime->sys_sec = 0;
  now = OSGetTime();
  OSTicksToCalendarTime(now, &calendarTime);
  sdate->sys_year = calendarTime.year;
  sdate->sys_month = calendarTime.mon + 1;
  sdate->sys_day = calendarTime.mday;
  stime->sys_hour = calendarTime.hour;
  stime->sys_min = calendarTime.min;
  stime->sys_sec = calendarTime.sec;
}
