#ifndef _DOLPHIN_OSRESET
#define _DOLPHIN_OSRESET

#include <SDKVersion.h>


#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_RESETCODE_RESTART 0x80000000
#define OS_RESETCODE_SYSTEM 0x40000000

#define OS_RESETCODE_EXEC 0xC0000000
#define OS_RESETCODE_NETCONFIG 0xC0010000

#define OS_RESET_TIMEOUT OSMillisecondsToTicks(1000)

#define OS_RESET_RESTART 0
#define OS_RESET_HOTRESET 1
#define OS_RESET_SHUTDOWN 2

#define OS_RESET_PRIO_SO 110
#define OS_RESET_PRIO_IP 111
#define OS_RESET_PRIO_CARD 127
#define OS_RESET_PRIO_PAD 127
#define OS_RESET_PRIO_GX 127
#define OS_RESET_PRIO_ALARM 4294967295

typedef BOOL (*OSResetFunction)(BOOL final);
void OSResetSystem(int reset, u32 resetCode, BOOL forceMenu);
typedef struct OSResetFunctionInfo OSResetFunctionInfo;

struct OSResetFunctionInfo {
  // public
  OSResetFunction func;
  u32 priority;

  // private
  OSResetFunctionInfo* next;
  OSResetFunctionInfo* prev;
};

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04_OS_APR && SDK_VERSION <= SDK_VERSION_RVL_2005_11

void OSResetSystem(int reset, u32 resetCode, BOOL forceMenu);

#elif SDK_VERSION >= SDK_VERSION_RVL_2008_01
typedef BOOL (*OSShutdownFunction)(BOOL final, u32 event);
typedef struct OSShutdownFunctionInfo OSShutdownFunctionInfo;

struct OSShutdownFunctionInfo {
  // public
  OSShutdownFunction func;
  u32 priority;

  // private
  OSShutdownFunctionInfo* next;
  OSShutdownFunctionInfo* prev;
};

void OSRegisterShutdownFunction(OSShutdownFunctionInfo* info);

#define OS_SETTING_CALENDAR 1u
#define OS_SETTING_DISPLAY 2u
#define OS_SETTING_SOUND 3u
#define OS_SETTING_PARENTAL 4u
#define OS_SETTING_INTERNET 5u
#define OS_SETTING_WC24 6u
#define OS_SETTING_UPDATE 7u

#define OS_SYSLAUNCH_URL_CALENDAR "Calendar/Calendar_index.html"
#define OS_SYSLAUNCH_URL_DISPLAY "Display/Display_index.html"
#define OS_SYSLAUNCH_URL_SOUND "Sound/Sound_index.html"
#define OS_SYSLAUNCH_URL_PARENTAL "Parental_Control/Parental_Control_index.html"
#define OS_SYSLAUNCH_URL_INTERNET "Internet/Internet_index.html"
#define OS_SYSLAUNCH_URL_WC24 "WiiConnect24/Wiiconnect24_index.html"
#define OS_SYSLAUNCH_URL_UPDATE "Update/Update_index.html"

void OSRebootSystem(void);
void OSShutdownSystem(void);
void OSRestart(u32 resetCode);
void OSReturnToMenu(void);
void OSReturnToDataManager(void);
void OSReturnToSetting(u8 setting);
u32 OSGetResetCode(void);
u32 OSSetBootDol(u32 dolOffset);

#define OSIsRestart() ((OSGetResetCode() & 0x80000000) ? TRUE : FALSE)

BOOL __OSCallShutdownFunctions(BOOL final, u32 event);
void __OSShutdownDevices(u32 event);
void __OSReturnToMenu(u8 menuMode);
void __OSReturnToMenul(u32 menuMode, const char* url, ...);
void __OSReturnToMenuForError(void);
void __OSHotResetForError(void);

void OSResetSystem(int reset, u32 resetCode, BOOL forceMenu);

#endif
#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_OSRESET
