#include <SDKVersion.h>
#include "dolphin/os.h"
#include "dolphin/DVDPriv.h"
#include "dolphin/db.h"
#include "dolphin/os/OSBootInfo.h"

extern OSTime __OSGetSystemTime();
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2002_09
static const char* __OSVersion =
    "<< Dolphin SDK - OS\trelease build: Sep  5 2002 05:32:39 (0x2301) >>";
#else
static const char* __OSVersion =
    "<< Dolphin SDK - OS\trelease build: Nov  5 2002 12:52:29 (0x2301) >>";
#endif
extern char _db_stack_end[];

#define OS_BI2_DEBUG_ADDRESS 0x800000F4
#define DEBUGFLAG_ADDR 0x800030E8
#define OS_DEBUG_ADDRESS_2 0x800030E9
#define OS_CURRENTCONTEXT_PADDR 0x00C0

extern char* __OSResetSWInterruptHandler[];

vu16 __OSDeviceCode : (OS_BASE_CACHED | 0x30E6);
ATTRIBUTE_ALIGN_DECL(32, static DVDDriveInfo DriveInfo);
static DVDCommandBlock DriveBlock;

static OSBootInfo* BootInfo;
static u32* BI2DebugFlag;
static u32* BI2DebugFlagHolder;
__declspec(weak) BOOL __OSIsGcam = FALSE;
static f64 ZeroF;
static f32 ZeroPS[2];
static BOOL AreWeInitialized = FALSE;
static __OSExceptionHandler* OSExceptionTable;
OSTime __OSStartTime;
BOOL __OSInIPL;
#else
#if SDK_VERSION == SDK_VERSION_DOLPHIN_2004_04
static const char* __OSVersion =
    "<< Dolphin SDK - OS\trelease build: May 21 2004 09:28:09 (0x2301) >>";
#else
const char* __OSVersion = "<< RVL_SDK - OS \trelease build: Jan 30 2008 01:38:43 (0x4199_60831) >>";
#endif
extern char _db_stack_end[];

#define OS_BI2_DEBUG_ADDRESS 0x800000F4
#define DEBUGFLAG_ADDR 0x800030E8
#define OS_DEBUG_ADDRESS_2 0x800030E9
#define OS_CURRENTCONTEXT_PADDR 0x00C0

extern char* __OSResetSWInterruptHandler[];

vu16 __OSDeviceCode : (OS_BASE_CACHED | 0x30E6);
#if SDK_VERSION == SDK_VERSION_DOLPHIN_2004_04
static DVDDriveInfo DriveInfo ATTRIBUTE_ALIGN(32);
static DVDCommandBlock DriveBlock;

static OSBootInfo* BootInfo;
static u32* BI2DebugFlag;
static u32 BI2DebugFlagHolder;
__declspec(weak) BOOL __OSIsGcam = FALSE;
static f64 ZeroF;
static f32 ZeroPS[2];
static BOOL AreWeInitialized = FALSE;
static __OSExceptionHandler* OSExceptionTable;
OSTime __OSStartTime;
BOOL __OSInIPL;
OSExecParams __OSRebootParams;

#else
OSExecParams __OSRebootParams;
static DVDDriveInfo DriveInfo ATTRIBUTE_ALIGN(32);
static DVDCommandBlock DriveBlock;

OSTime __OSStartTime;
static OSBootInfo* BootInfo;
static u32* BI2DebugFlag;
static u32 BI2DebugFlagHolder;

static char GameNameBuffer[5];

static f64 ZeroF;
static f32 ZeroPS[2];

static __OSExceptionHandler* OSExceptionTable;

static BOOL AreWeInitialized;
__declspec(weak) BOOL __OSIsGcam;
BOOL __OSInNandBoot;
BOOL __OSInIPL;
extern BOOL __OSInReboot;
#endif
#endif

extern u8 __ArenaHi[];
extern u8 __ArenaLo[];
extern u32 __DVDLongFileNameFlag;
extern u32 __PADSpec;

#define OS_EXCEPTIONTABLE_ADDR 0x3000
#define OS_DBJUMPPOINT_ADDR 0x60
// memory locations for important stuff
#define OS_CACHED_REGION_PREFIX 0x8000
#define OS_BI2_DEBUG_ADDRESS 0x800000F4
#define OS_BI2_DEBUGFLAG_OFFSET 0xC
#define PAD3_BUTTON_ADDR 0x800030E4
#define OS_DVD_DEVICECODE 0x800030E6
#define DEBUGFLAG_ADDR 0x800030E8
#define OS_DEBUG_ADDRESS_2 0x800030E9
#define DB_EXCEPTIONRET_OFFSET 0xC
#define DB_EXCEPTIONDEST_OFFSET 0x8
#define MSR_RI_BIT 0x1E

void OSDefaultExceptionHandler(__OSException exception, OSContext* context);
extern BOOL __DBIsExceptionMarked(__OSException);
static void OSExceptionInit(void);
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
void __OSGetExecParams(OSExecParams* params);
void EnableMetroTRKInterrupts(void);
extern char _stack_addr[];

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
u8 OSGetAppType(void);
void __OSInitAlarm(void);
void __OSInitIPCBuffer(void);
void IPCCltInit(void);
void __OSInitSTM(void);
void SCInit(void);
u32 SCCheckStatus(void);
void __OSInitNet(void);
BOOL __DVDCheckDevice(void);
void OSReturnToMenu(void);
void __OSInitPlayTime(void);
void __OSStartPlayRecord(void);
void __OSInitMemoryProtection(void);
#endif

static void DisableWriteGatherPipe(void) {
  u32 hid2;

  hid2 = PPCMfhid2();
  hid2 &= ~0x40000000;
  PPCMthid2(hid2);
}
#endif

/* clang-format off */
asm void __OSFPRInit(void)
{
    nofralloc

    mfmsr   r3
    ori     r3, r3, 0x2000
    mtmsr   r3

    mfspr   r3, 0x398
    rlwinm. r3, r3, 3, 31, 31
    beq     SkipPairedSingles

    lis     r3, ZeroPS@ha
    addi    r3, r3, ZeroPS@l
    psq_l   fp0, 0(r3), 0, 0
    ps_mr   fp1, fp0
    ps_mr   fp2, fp0
    ps_mr   fp3, fp0
    ps_mr   fp4, fp0
    ps_mr   fp5, fp0
    ps_mr   fp6, fp0
    ps_mr   fp7, fp0
    ps_mr   fp8, fp0
    ps_mr   fp9, fp0
    ps_mr   fp10, fp0
    ps_mr   fp11, fp0
    ps_mr   fp12, fp0
    ps_mr   fp13, fp0
    ps_mr   fp14, fp0
    ps_mr   fp15, fp0
    ps_mr   fp16, fp0
    ps_mr   fp17, fp0
    ps_mr   fp18, fp0
    ps_mr   fp19, fp0
    ps_mr   fp20, fp0
    ps_mr   fp21, fp0
    ps_mr   fp22, fp0
    ps_mr   fp23, fp0
    ps_mr   fp24, fp0
    ps_mr   fp25, fp0
    ps_mr   fp26, fp0
    ps_mr   fp27, fp0
    ps_mr   fp28, fp0
    ps_mr   fp29, fp0
    ps_mr   fp30, fp0
    ps_mr   fp31, fp0

SkipPairedSingles:
    lfd     fp0, ZeroF
    fmr     fp1, fp0
    fmr     fp2, fp0
    fmr     fp3, fp0
    fmr     fp4, fp0
    fmr     fp5, fp0
    fmr     fp6, fp0
    fmr     fp7, fp0
    fmr     fp8, fp0
    fmr     fp9, fp0
    fmr     fp10, fp0
    fmr     fp11, fp0
    fmr     fp12, fp0
    fmr     fp13, fp0
    fmr     fp14, fp0
    fmr     fp15, fp0
    fmr     fp16, fp0
    fmr     fp17, fp0
    fmr     fp18, fp0
    fmr     fp19, fp0
    fmr     fp20, fp0
    fmr     fp21, fp0
    fmr     fp22, fp0
    fmr     fp23, fp0
    fmr     fp24, fp0
    fmr     fp25, fp0
    fmr     fp26, fp0
    fmr     fp27, fp0
    fmr     fp28, fp0
    fmr     fp29, fp0
    fmr     fp30, fp0
    fmr     fp31, fp0

    mtfsf   0xFF, fp0

    blr
}
/* clang-format on */

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
u32 OSGetConsoleType() {
  if (BootInfo == NULL || BootInfo->consoleType == 0) {
    return OS_CONSOLE_ARTHUR;
  }
  return BootInfo->consoleType;
}

void* __OSSavedRegionStart;
void* __OSSavedRegionEnd;

extern u32 BOOT_REGION_START : 0x812FDFF0; //(*(u32 *)0x812fdff0)
extern u32 BOOT_REGION_END : 0x812FDFEC;   //(*(u32 *)0x812fdfec)

void ClearArena(void) {
  if ((u32)(OSGetResetCode() + 0x80000000) != 0U) {
    __OSSavedRegionStart = 0U;
    __OSSavedRegionEnd = 0U;
    memset(OSGetArenaLo(), 0U, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
    return;
  }
  __OSSavedRegionStart = (void*)BOOT_REGION_START;
  __OSSavedRegionEnd = (void*)BOOT_REGION_END;
  if (BOOT_REGION_START == 0U) {
    memset(OSGetArenaLo(), 0U, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
    return;
  }

  if ((u32)OSGetArenaLo() < (u32)__OSSavedRegionStart) {
    if ((u32)OSGetArenaHi() <= (u32)__OSSavedRegionStart) {
      memset((u32)OSGetArenaLo(), 0U, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
      return;
    }
    memset(OSGetArenaLo(), 0U, (u32)__OSSavedRegionStart - (u32)OSGetArenaLo());
    if ((u32)OSGetArenaHi() > (u32)__OSSavedRegionEnd) {
      memset((u32)__OSSavedRegionEnd, 0, (u32)OSGetArenaHi() - (u32)__OSSavedRegionEnd);
    }
  }
}
#elif SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
u32 OSGetConsoleType() {
  if (BootInfo == NULL || BootInfo->consoleType == 0) {
    return OS_CONSOLE_ARTHUR;
  }
  return BootInfo->consoleType;
}

static void ClearArena(void) {
  if (!((OSGetResetCode() & 0x80000000) ? TRUE : FALSE)) {
    memset(OSGetArenaLo(), 0, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
    return;
  }

  if (*(u32*)&__OSRebootParams.regionStart == 0) {
    memset(OSGetArenaLo(), 0, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
    return;
  }

  if ((u32)OSGetArenaLo() < *(u32*)&__OSRebootParams.regionStart) {
    if ((u32)OSGetArenaHi() <= *(u32*)&__OSRebootParams.regionStart) {
      memset(OSGetArenaLo(), 0, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
      return;
    }

    memset(OSGetArenaLo(), 0, *(u32*)&__OSRebootParams.regionStart - (u32)OSGetArenaLo());

    if ((u32)OSGetArenaHi() > (u32)__OSRebootParams.regionEnd) {
      memset(__OSRebootParams.regionEnd, 0,
             (u32)OSGetArenaHi() - (u32)__OSRebootParams.regionEnd);
    }
  }
}
#else
#define OS_MEM1_BEGIN 0x80000000
#define OSIsMEM1Region(addr) (((u32)(addr)&0x30000000) == 0)
#define OSIsMEM2Region(addr) (((u32)(addr)&0x30000000) == 0x10000000)

void __OSGetIOSRev(OSIOSRev* rev) {
  u32 version = *(u32*)OSPhysicalToUncached(0x3140);
  u32 builddate = *(u32*)OSPhysicalToUncached(0x3144);

  rev->idHi = version >> 24 & 0xFF;
  rev->idLo = version >> 16 & 0xFF;
  rev->verMajor = version >> 8 & 0xFF;
  rev->verMinor = version >> 0 & 0xFF;

  rev->buildMon = (builddate >> 16 & 0xF) + (builddate >> 20 & 0xF) * 10;
  rev->buildDay = (builddate >> 8 & 0xF) + (builddate >> 12 & 0xF) * 10;
  rev->buildYear = (builddate >> 0 & 0xF) + (builddate >> 4 & 0xF) * 10 + 2000;
}

u32 OSGetConsoleType() {
  u32 hollywood;
  u32 mem2size;

  if (BootInfo == NULL || BootInfo->consoleType == 0) {
    return 0x10000002;
  }

  hollywood = *(u32*)OSPhysicalToCached(0x3138);

  if (__OSDeviceCode & 0x8000) {
    switch (__OSDeviceCode & ~0x8000) {
    case 0x0002:
    case 0x0003:
    case 0x0203:
      switch (hollywood) {
      case 0x00000000:
        return 0x00000010;
      case 0x00000001:
        return 0x00000011;
      case 0x00000002:
        return 0x00000012;
      case 0x00000010:
        return 0x00000020;
      case 0x00000011:
        return 0x00000021;
      }

      if (hollywood > 0x00000011) {
        return 0x00000021;
      }

    case 0x0202:
    case 0x0201:
      switch (hollywood) {
      case 0x00000000:
        return 0x10000010;
      case 0x00000001:
        return 0x10000011;
      case 0x00000002:
        return 0x10000012;
      case 0x00000010:
        return 0x10000020;
      case 0x00000011:
        return 0x10000021;
      }

      if (hollywood > 0x00000011) {
        return 0x10000021;
      }
    case 0x0300:
      return 0x00000100;
    }
  }

  mem2size = OSGetPhysicalMem2Size();

  switch (hollywood) {
  case 0x00000000:
    if (mem2size == 0x4000000) {
      return 0x00000010;
    }
    return 0x10000010;
  case 0x00000001:
    if (mem2size == 0x4000000) {
      return 0x00000011;
    }
    return 0x10000011;
  case 0x00000002:
    if (mem2size == 0x4000000) {
      return 0x00000012;
    }
    return 0x10000012;
  case 0x00000010:
    if (mem2size == 0x4000000) {
      return 0x00000020;
    }
    return 0x10000020;
  case 0x00000011:
    if (mem2size == 0x4000000) {
      return 0x00000021;
    }
    return 0x10000021;
  }

  if (hollywood > 0x00000011) {
    if (mem2size == 0x4000000) {
      return 0x00000021;
    }
    return 0x10000021;
  }

  return BootInfo->consoleType;
}

static void MemClear(void* mem, u32 size) {
  void* flush = (0x40000 < size) ? (u8*)((u32)mem + (size - 0x40000)) : mem;
  DCZeroRange(mem, size);
  DCFlushRange(flush, 0x40000);
}

static void ClearArena(void) __attribute__((never_inline)) {
  if (!((OSGetResetCode() >> 31) & 1)) {
    MemClear(OSGetArenaLo(), (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
  } else if (__OSRebootParams.regionStart == NULL || !OSIsMEM1Region(__OSRebootParams.regionStart)) {
    MemClear(OSGetArenaLo(), (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
  } else if (OSGetArenaLo() < __OSRebootParams.regionStart) {
    if (OSGetArenaHi() <= __OSRebootParams.regionStart) {
      MemClear(OSGetArenaLo(), (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
    } else {
      MemClear(OSGetArenaLo(), (u32)__OSRebootParams.regionStart - (u32)OSGetArenaLo());

      if (OSGetArenaHi() > __OSRebootParams.regionEnd) {
        MemClear(__OSRebootParams.regionEnd, (u32)OSGetArenaHi() - (u32)__OSRebootParams.regionEnd);
      }
    }
  }
}

static void ClearMEM2Arena(void) __attribute__((never_inline)) {
  if (!((OSGetResetCode() >> 31) & 1)) {
    MemClear(OSGetMEM2ArenaLo(), (u32)OSGetMEM2ArenaHi() - (u32)OSGetMEM2ArenaLo());
  } else if (__OSRebootParams.regionStart == NULL || !OSIsMEM2Region(__OSRebootParams.regionStart)) {
    MemClear(OSGetMEM2ArenaLo(), (u32)OSGetMEM2ArenaHi() - (u32)OSGetMEM2ArenaLo());
  } else if (OSGetMEM2ArenaLo() < __OSRebootParams.regionStart) {
    if (OSGetMEM2ArenaHi() <= __OSRebootParams.regionStart) {
      MemClear(OSGetMEM2ArenaLo(), (u32)OSGetMEM2ArenaHi() - (u32)OSGetMEM2ArenaLo());
    } else {
      MemClear(OSGetMEM2ArenaLo(), (u32)__OSRebootParams.regionStart - (u32)OSGetMEM2ArenaLo());
      if (OSGetMEM2ArenaHi() > __OSRebootParams.regionEnd) {
        MemClear(__OSRebootParams.regionEnd,
                 (u32)OSGetMEM2ArenaHi() - (u32)__OSRebootParams.regionEnd);
      }
    }
  }
}

#endif

static void InquiryCallback(s32 result, DVDCommandBlock* block) {
  switch (block->state) {
  case 0:
    __OSDeviceCode = (u16)(0x8000 | DriveInfo.deviceCode);
    break;
  default:
    __OSDeviceCode = 1;
    break;
  }
}

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
void OSInit(void) {
  /*
  Initializes the Dolphin operating system.
      - most of the main operations get farmed out to other functions
      - loading debug info and setting up heap bounds largely happen here
      - a lot of OS reporting also gets controlled here
  */
  // pretty sure this is the min(/max) amount of pointers etc for the stack to match
  BI2Debug* DebugInfo;
  void* debugArenaLo;
  u32 inputConsoleType;
  u32 tdev;

  // check if we've already done all this or not
  if ((BOOL)AreWeInitialized == FALSE) { // fantastic name
    AreWeInitialized = TRUE;             // flag to make sure we don't have to do this again

    // SYSTEM //
    __OSStartTime = __OSGetSystemTime();
    OSDisableInterrupts();

    // set some PPC things
    PPCDisableSpeculation();
    PPCSetFpNonIEEEMode();

    // DEBUG //
    // load some DVD stuff
    BI2DebugFlag = 0;                       // debug flag from the DVD BI2 header
    BootInfo = (OSBootInfo*)OS_BASE_CACHED; // set pointer to BootInfo

    __DVDLongFileNameFlag = (u32)0; // flag to tell us whether we make it through the debug loading

    // time to grab a bunch of debug info from the DVD
    // the address for where the BI2 debug info is, is stored at OS_BI2_DEBUG_ADDRESS
    DebugInfo = (BI2Debug*)*((u32*)OS_BI2_DEBUG_ADDRESS);

    // if the debug info address exists, grab some debug info
    if (DebugInfo != NULL) {
      BI2DebugFlag = &DebugInfo->debugFlag;              // debug flag from DVD BI2
      __PADSpec = (u32)DebugInfo->padSpec;               // some other info from DVD BI2
      *((u8*)DEBUGFLAG_ADDR) = (u8)*BI2DebugFlag;        // store flag in mem
      *((u8*)OS_DEBUG_ADDRESS_2) = (u8)__PADSpec;        // store other info in mem
    } else if (BootInfo->arenaHi) {                      // if the top of the heap is already set
      BI2DebugFlagHolder = (u32*)*((u8*)DEBUGFLAG_ADDR); // grab whatever's stored at 0x800030E8
      BI2DebugFlag = (u32*)&BI2DebugFlagHolder;          // flag is then address of flag holder
      __PADSpec = (u32) * ((u8*)OS_DEBUG_ADDRESS_2);     // pad spec is whatever's at 0x800030E9
    }

    __DVDLongFileNameFlag = 1; // we made it through debug!

    // HEAP //
    // set up bottom of heap (ArenaLo)
    // grab address from BootInfo if it exists, otherwise use default __ArenaLo
    OSSetArenaLo((BootInfo->arenaLo == NULL) ? __ArenaLo : BootInfo->arenaLo);

    // if the input arenaLo is null, and debug flag location exists (and flag is < 2),
    //     set arenaLo to just past the end of the db stack
    if ((BootInfo->arenaLo == NULL) && (BI2DebugFlag != 0) && (*BI2DebugFlag < 2)) {
      debugArenaLo = (char*)(((u32)_db_stack_end + 0x1f) & ~0x1f);
      OSSetArenaLo(debugArenaLo);
    }

    // set up top of heap (ArenaHi)
    // grab address from BootInfo if it exists, otherwise use default __ArenaHi
    OSSetArenaHi((BootInfo->arenaHi == NULL) ? __ArenaHi : BootInfo->arenaHi);

    // OS INIT AND REPORT //
    // initialise a whole bunch of OS stuff
    OSExceptionInit();
    __OSInitSystemCall();
    OSInitAlarm();
    __OSModuleInit();
    __OSInterruptInit();
    __OSSetInterruptHandler(__OS_INTERRUPT_PI_RSW, (void*)__OSResetSWInterruptHandler);
    __OSContextInit();
    __OSCacheInit();
    EXIInit();
    SIInit();
    __OSInitSram();
    __OSThreadInit();
    __OSInitAudioSystem();
    PPCMthid2(PPCMfhid2() & 0xBFFFFFFF);
    if ((BOOL)__OSInIPL == FALSE) {
      __OSInitMemoryProtection();
    }

    // begin OS reporting
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2002_09
    OSReport("\nDolphin OS $Revision: 58 $.\n");
    OSReport("Kernel built : %s %s\n", "Sep  5 2002", "05:32:39");
#else
    OSReport("\nDolphin OS $Revision: 60 $.\n");
    OSReport("Kernel built : %s %s\n", "Nov  5 2002", "12:52:29");
#endif
    OSReport("Console Type : ");

    // this is a function in the same file, but it doesn't seem to match
    // inputConsoleType = OSGetConsoleType();

    // inputConsoleType = (BootInfo == NULL || (inputConsoleType = BootInfo->consoleType) == 0) ?
    // 0x10000002 : BootInfo->consoleType;
    if (BootInfo == NULL || (inputConsoleType = BootInfo->consoleType) == 0) {
      inputConsoleType = OS_CONSOLE_ARTHUR; // default console type
    } else {
      inputConsoleType = BootInfo->consoleType;
    }

    // work out what console type this corresponds to and report it
    // consoleTypeSwitchHi = inputConsoleType & 0xF0000000;
    switch (inputConsoleType & 0xffff0000) { // check "first" byte
    case OS_CONSOLE_RETAIL:
      OSReport("Retail %d\n", inputConsoleType);
      break;
    default:
      switch (inputConsoleType & 0x0000ffff) { // if "first" byte is 2, check "the rest"
      case OS_CONSOLE_EMULATOR:
        OSReport("Mac Emulator\n");
        break;
      case OS_CONSOLE_PC_EMULATOR:
        OSReport("PC Emulator\n");
        break;
      case OS_CONSOLE_ARTHUR:
        OSReport("EPPC Arthur\n");
        break;
      case OS_CONSOLE_MINNOW:
        OSReport("EPPC Minnow\n");
        break;
      default:
        tdev = ((u32)inputConsoleType & 0x0000ffff);
        OSReport("Development HW%d (%08x)\n", tdev - 3, inputConsoleType);
        break;
      }
      break;
    }

    // report memory size
    OSReport("Memory %d MB\n", (u32)BootInfo->memorySize >> 0x14U);
    // report heap bounds
    OSReport("Arena : 0x%x - 0x%x\n", OSGetArenaLo(), OSGetArenaHi());
    // report OS version
    OSRegisterVersion(__OSVersion);

    // if location of debug flag exists, and flag is >= 2, enable MetroTRKInterrupts
    if (BI2DebugFlag && ((*BI2DebugFlag) >= 2)) {
      EnableMetroTRKInterrupts();
    }

    // free up memory and re-enable things
    ClearArena();
    OSEnableInterrupts();

    // check if we can load OS from IPL; if not, grab it from DVD (?)
    if ((BOOL)__OSInIPL == FALSE) {
      DVDInit();
      if ((BOOL)__OSIsGcam) {
        __OSDeviceCode = 0x9000;
        return;
      }
      DCInvalidateRange(&DriveInfo, sizeof(DriveInfo));
      DVDInquiryAsync((char*)&DriveBlock, &DriveInfo, InquiryCallback);
    }
  }
}
#elif SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
void OSInit(void) {
  u32 consoleType;
  void* bi2StartAddr;

  if (AreWeInitialized == FALSE) {
    AreWeInitialized = TRUE;

    __OSStartTime = __OSGetSystemTime();
    OSDisableInterrupts();

    __OSGetExecParams(&__OSRebootParams);
    PPCMtmmcr0(0);
    PPCMtmmcr1(0);
    PPCMtpmc1(0);
    PPCMtpmc2(0);
    PPCMtpmc3(0);
    PPCMtpmc4(0);
    PPCDisableSpeculation();
    PPCSetFpNonIEEEMode();

    BootInfo = (OSBootInfo*)OSPhysicalToCached(0);
    BI2DebugFlag = 0;
    __DVDLongFileNameFlag = 0;

    bi2StartAddr = (void*)(*(u32*)OSPhysicalToCached(0xF4));
    if (bi2StartAddr) {
      BI2DebugFlag = (void*)((char*)bi2StartAddr + 0xC);
      __PADSpec = ((u32*)bi2StartAddr)[9];
      *(u8*)OSPhysicalToCached(0x30E8) = *BI2DebugFlag;
      *(u8*)OSPhysicalToCached(0x30E9) = __PADSpec;
    } else if (BootInfo->arenaHi) {
      BI2DebugFlagHolder = *(u8*)OSPhysicalToCached(0x30E8);
      BI2DebugFlag = &BI2DebugFlagHolder;
      __PADSpec = *(u8*)OSPhysicalToCached(0x30E9);
    }

    __DVDLongFileNameFlag = 1;

    OSSetArenaLo((!BootInfo->arenaLo) ? &__ArenaLo : BootInfo->arenaLo);
    if ((!BootInfo->arenaLo) && (BI2DebugFlag) && (*(u32*)BI2DebugFlag < 2)) {
      OSSetArenaLo((void*)(((u32)(char*)&_stack_addr + 0x1F) & 0xFFFFFFE0));
    }
    OSSetArenaHi((!BootInfo->arenaHi) ? &__ArenaHi : BootInfo->arenaHi);

    OSExceptionInit();
    __OSInitSystemCall();
    OSInitAlarm();
    __OSModuleInit();
    __OSInterruptInit();
    __OSSetInterruptHandler(__OS_INTERRUPT_PI_RSW, (void*)__OSResetSWInterruptHandler);
    __OSContextInit();
    __OSCacheInit();
    EXIInit();
    SIInit();
    __OSInitSram();
    __OSThreadInit();
    __OSInitAudioSystem();

    DisableWriteGatherPipe();

    if (!__OSInIPL) {
      __OSInitMemoryProtection();
    }

    OSReport("\nDolphin OS\n");
    OSReport("Kernel built : %s %s\n", "May 21 2004", "09:28:09");
    OSReport("Console Type : ");

    consoleType = OSGetConsoleType();
    switch (consoleType & 0xF0000000) {
    case OS_CONSOLE_RETAIL:
      OSReport("Retail %d\n", consoleType);
      break;
    case OS_CONSOLE_DEVELOPMENT:
    case OS_CONSOLE_TDEV:
      switch (consoleType & 0x0FFFFFFF) {
      case OS_CONSOLE_EMULATOR:
        OSReport("Mac Emulator\n");
        break;
      case OS_CONSOLE_PC_EMULATOR:
        OSReport("PC Emulator\n");
        break;
      case OS_CONSOLE_ARTHUR:
        OSReport("EPPC Arthur\n");
        break;
      case OS_CONSOLE_MINNOW:
        OSReport("EPPC Minnow\n");
        break;
      default:
        OSReport("Development HW%d (%08x)\n", (consoleType & 0xFFFFFFF) - 3, consoleType);
        break;
      }
      break;
    default:
      OSReport("%08x\n", consoleType);
      break;
    }

    OSReport("Memory %d MB\n", (u32)BootInfo->memorySize >> 0x14U);
    OSReport("Arena : 0x%x - 0x%x\n", OSGetArenaLo(), OSGetArenaHi());
    OSRegisterVersion(__OSVersion);

    if (BI2DebugFlag && ((*BI2DebugFlag) >= 2)) {
      EnableMetroTRKInterrupts();
    }

    ClearArena();
    OSEnableInterrupts();

    if (!__OSInIPL) {
      DVDInit();

      if (__OSIsGcam) {
        __OSDeviceCode = 0x9000;
        return;
      }

      DCInvalidateRange(&DriveInfo, sizeof(DriveInfo));
      DVDInquiryAsync(&DriveBlock, &DriveInfo, InquiryCallback);
    }
  }
}

#else

static void ReportOSInfo(void) {
  u32 type;
  OSIOSRev rev;
  u32 category;
  u32 tdev;

  OSReport("\nRevolution OS\n");
  OSReport("Kernel built : %s %s\n", "Jan 30 2008", "01:38:43");

  OSReport("Console Type : ");
  type = OSGetConsoleType();
  category = type & 0xF0000000;
  switch (category) {
  case 0x00000000:
    switch (type) {
    case 0x00000011:
      OSReport("Pre-production board 1\n");
      break;
    case 0x00000012:
      OSReport("Pre-production board 2-1\n");
      break;
    case 0x00000020:
      OSReport("Pre-production board 2-2\n");
      break;
    case 0x00000100:
      OSReport("RVA 1\n");
      break;
    default:
      OSReport("Retail %d\n", type);
      break;
    }
    break;
  case 0x10000000:
    switch (type) {
    case 0x10000021:
      OSReport("NDEV 2.1\n");
      break;
    case 0x10000020:
      OSReport("NDEV 2.0\n");
      break;
    case 0x10000012:
      OSReport("NDEV 1.2\n");
      break;
    case 0x10000011:
      OSReport("NDEV 1.1\n");
      break;
    case 0x10000010:
      OSReport("NDEV 1.0\n");
      break;
    case 0x10000008:
      OSReport("Revolution Emulator\n");
      break;
    default:
      OSReport("Emulation platform (%08x)\n", type);
      break;
    }
    break;
  case 0x20000000:
    tdev = type & 0x0FFFFFFF;
    OSReport("TDEV-based emulation HW%d\n", tdev - 3);
    break;
  default:
    OSReport("%08x\n", type);
    break;
  }

  __OSGetIOSRev(&rev);
  OSReport("Firmware     : %d.%d.%d ", rev.idLo, rev.verMajor, rev.verMinor);
  OSReport("(%d/%d/%d)\n", rev.buildMon, rev.buildDay, rev.buildYear);

  OSReport("Memory %d MB\n",
           (OSGetConsoleSimulatedMem1Size() + OSGetConsoleSimulatedMem2Size()) >> 20);
  OSReport("MEM1 Arena : 0x%x - 0x%x\n", OSGetMEM1ArenaLo(), OSGetMEM1ArenaHi());
  OSReport("MEM2 Arena : 0x%x - 0x%x\n", OSGetMEM2ArenaLo(), OSGetMEM2ArenaHi());
}

void OSInit(void) {
  void* bi2StartAddr;
  void* mem1lo;
  void* mem1hi;
  void* mem2lo;
  void* mem2hi;

  if (!AreWeInitialized) {
    AreWeInitialized = TRUE;

    __OSStartTime = __OSGetSystemTime();
    OSDisableInterrupts();
    __OSGetExecParams(&__OSRebootParams);

    PPCMtmmcr0(0);
    PPCMtmmcr1(0);
    PPCMtpmc1(0);
    PPCMtpmc2(0);
    PPCMtpmc3(0);
    PPCMtpmc4(0);
    PPCMthid4(0x83900000);
    PPCDisableSpeculation();
    PPCSetFpNonIEEEMode();

    BootInfo = (OSBootInfo*)OSPhysicalToCached(0);
    BI2DebugFlag = NULL;
    __DVDLongFileNameFlag = FALSE;

    bi2StartAddr = *(void**)OSPhysicalToCached(0xF4);
    if (bi2StartAddr != NULL) {
      BI2DebugFlag = (u32*)((char*)bi2StartAddr + 0xC);
      __PADSpec = ((u32*)bi2StartAddr)[9];
      *(u8*)OSPhysicalToCached(0x30E8) = *BI2DebugFlag;
      *(u8*)OSPhysicalToCached(0x30E9) = __PADSpec;
    } else if (BootInfo->arenaHi != NULL) {
      BI2DebugFlagHolder = *(u8*)OSPhysicalToCached(0x30E8);
      BI2DebugFlag = &BI2DebugFlagHolder;
      __PADSpec = *(u8*)OSPhysicalToCached(0x30E9);
    }

    __DVDLongFileNameFlag = TRUE;

    mem1lo = *(void**)OSPhysicalToCached(0x310C);
    if (mem1lo == NULL) {
      if (OSIsMEM1Region(__ArenaLo)) {
        mem1lo = BootInfo->arenaLo == NULL ? (void*)__ArenaLo : BootInfo->arenaLo;

        if (BootInfo->arenaLo == NULL && BI2DebugFlag != NULL && *BI2DebugFlag < 2) {
          mem1lo = (void*)(((u32)_stack_addr + 0x1F) & ~0x1F);
        }
      } else {
        mem1lo = (void*)0x80004000;
      }
    }
    OSSetMEM1ArenaLo(mem1lo);

    mem1hi = *(void**)OSPhysicalToCached(0x3110);
    if (mem1hi == NULL) {
      mem1hi = BootInfo->arenaHi == NULL ? (void*)__ArenaHi : BootInfo->arenaHi;
    }
    OSSetMEM1ArenaHi(mem1hi);

    mem2lo = *(void**)OSPhysicalToCached(0x3124);
    if (mem2lo != NULL) {
      if (OSIsMEM2Region(__ArenaLo)) {
        mem2lo = __ArenaLo;

        if (BI2DebugFlag != NULL && *BI2DebugFlag < 2) {
          mem2lo = (void*)(((u32)_stack_addr + 0x1F) & ~0x1F);
        }
      } else if (mem2lo >= (void*)0x90000000 && mem2lo < (void*)0x90000800) {
        mem2lo = (void*)0x90000800;
      }
      OSSetMEM2ArenaLo(mem2lo);
    }

    mem2hi = *(void**)OSPhysicalToCached(0x3128);
    if (mem2hi != NULL) {
      OSSetMEM2ArenaHi(mem2hi);
    }

    __OSInitIPCBuffer();
    OSExceptionInit();
    __OSInitSystemCall();
    __OSInitAlarm();
    __OSModuleInit();
    __OSInterruptInit();
    __OSContextInit();
    __OSCacheInit();
    EXIInit();
    SIInit();
    __OSInitSram();
    __OSThreadInit();
    __OSInitAudioSystem();

    DisableWriteGatherPipe();

    if (!__OSInIPL) {
      __OSInitMemoryProtection();
    }

    ReportOSInfo();
    OSRegisterVersion(__OSVersion);

    if (BI2DebugFlag != NULL && *BI2DebugFlag >= 2) {
      EnableMetroTRKInterrupts();
    }

    if (!__OSInNandBoot && !__OSInReboot) {
      ClearArena();
      ClearMEM2Arena();
    }
    OSEnableInterrupts();
    IPCCltInit();

    if (!__OSInNandBoot && !__OSInReboot) {
      __OSInitSTM();

      SCInit();
      while (SCCheckStatus() == 1) {
        ;
      }

      __OSInitNet();
    }

    if (!__OSInIPL) {
      switch (*(u8*)OSPhysicalToCached(0x315C)) {
      case 0x81:
        OSReport("OS ERROR: boot program is not for RVL target. Please use "
                 "correct boot program.\n");
        OSPanic("OS.c", 1150, "Failed to run app");
        break;
      case 0x80:
      default:
        break;
      }

      switch (*(u8*)OSPhysicalToCached(0x315D)) {
      case 0x81:
        OSReport("OS ERROR: apploader[D].img is not for RVL target. Please use "
                 "correct apploader[D].img.\n");
        OSPanic("OS.c", 1168, "Failed to run app");
        break;
      case 0x80:
      default:
        break;
      }

      DVDInit();

      if (__OSIsGcam) {
        __OSDeviceCode = 0x9000;
      } else if (__OSDeviceCode == 0) {
        DCInvalidateRange(&DriveInfo, sizeof(DVDDriveInfo));
        DVDInquiryAsync(&DriveBlock, &DriveInfo, InquiryCallback);
      }

      if (OSGetAppType() == 0x80 && !__OSInReboot) {
        if (!__DVDCheckDevice()) {
          OSReturnToMenu();
        }
      }
    }

    if (!__OSInIPL && !__OSInNandBoot) {
      __OSInitPlayTime();
    }

    if (!__OSInIPL && !__OSInNandBoot && !__OSInReboot) {
      __OSStartPlayRecord();
    }
  }
}

#endif
static u32 __OSExceptionLocations[] = {
    0x00000100, 0x00000200, 0x00000300, 0x00000400, 0x00000500, 0x00000600, 0x00000700, 0x00000800,
    0x00000900, 0x00000C00, 0x00000D00, 0x00000F00, 0x00001300, 0x00001400, 0x00001700,
};

// dummy entry points to the OS Exception vector
void __OSEVStart(void);
void __OSEVEnd(void);
void __OSEVSetNumber(void);
void __OSExceptionVector(void);

void __DBVECTOR(void);
void __OSDBINTSTART(void);
void __OSDBINTEND(void);
void __OSDBJUMPSTART(void);
void __OSDBJUMPEND(void);
void __OSDBJUMPDEST(void);

#define NOP 0x60000000

__OSExceptionHandler __OSSetExceptionHandler(__OSException exception, __OSExceptionHandler handler);

/*
 * --INFO--
 * Address:	800EB654
 * Size:	000280
 */
static void OSExceptionInit(void) {
  __OSException exception;
  void* destAddr;

  // These two vars help us change the exception number embedded
  // in the exception handler code.
  u32* opCodeAddr;
  u32 oldOpCode;

  // Address range of the actual code to be copied.
  u8* handlerStart;
  u32 handlerSize;

  // Install the first level exception vector.
  opCodeAddr = (u32*)__OSEVSetNumber;
  oldOpCode = *opCodeAddr;
  handlerStart = (u8*)__OSEVStart;
  handlerSize = (u32)((u8*)__OSEVEnd - (u8*)__OSEVStart);

  // Install the DB integrator, only if we are the first OSInit to be run
  destAddr = (void*)OSPhysicalToCached(OS_DBJUMPPOINT_ADDR);
  if (*(u32*)destAddr == 0) // Lomem should be zero cleared only once by BS2
  {
    DBPrintf("Installing OSDBIntegrator\n");
    memcpy(destAddr, (void*)__OSDBINTSTART, (u32)__OSDBINTEND - (u32)__OSDBINTSTART);
    DCFlushRangeNoSync(destAddr, (u32)__OSDBINTEND - (u32)__OSDBINTSTART);
    __sync();
    ICInvalidateRange(destAddr, (u32)__OSDBINTEND - (u32)__OSDBINTSTART);
  }

  // Copy the right vector into the table
  for (exception = 0; exception < __OS_EXCEPTION_MAX; exception++) {
    if (BI2DebugFlag && (*BI2DebugFlag >= 2) && __DBIsExceptionMarked(exception)) {
      // this DBPrintf is suspicious.
      DBPrintf(">>> OSINIT: exception %d commandeered by TRK\n", exception);
      continue;
    }

    // Modify the copy of code in text before transferring
    // to the exception table.
    *opCodeAddr = oldOpCode | exception;

    // Modify opcodes at __DBVECTOR if necessary
    if (__DBIsExceptionMarked(exception)) {
      DBPrintf(">>> OSINIT: exception %d vectored to debugger\n", exception);
      memcpy((void*)__DBVECTOR, (void*)__OSDBJUMPSTART, (u32)__OSDBJUMPEND - (u32)__OSDBJUMPSTART);
    } else {
      // make sure the opcodes are still nop
      u32* ops = (u32*)__DBVECTOR;
      int cb;

      for (cb = 0; cb < (u32)__OSDBJUMPEND - (u32)__OSDBJUMPSTART; cb += sizeof(u32)) {
        *ops++ = NOP;
      }
    }

    // Install the modified handler.
    destAddr = (void*)OSPhysicalToCached(__OSExceptionLocations[(u32)exception]);
    memcpy(destAddr, handlerStart, handlerSize);
    DCFlushRangeNoSync(destAddr, handlerSize);
    __sync();
    ICInvalidateRange(destAddr, handlerSize);
  }

  // initialize pointer to exception table
  OSExceptionTable = OSPhysicalToCached(OS_EXCEPTIONTABLE_ADDR);

  // install default exception handlers
  for (exception = 0; exception < __OS_EXCEPTION_MAX; exception++) {
    __OSSetExceptionHandler(exception, OSDefaultExceptionHandler);
  }

  // restore the old opcode, so that we can re-start an application without
  // downloading the text segments
  *opCodeAddr = oldOpCode;

  DBPrintf("Exceptions initialized...\n");
}

static asm void __OSDBIntegrator(void) {
  /* clang-format off */
    nofralloc
entry __OSDBINTSTART
    li      r5, OS_DBINTERFACE_ADDR
    mflr    r3
    stw     r3, DB_EXCEPTIONRET_OFFSET(r5)
    lwz     r3, DB_EXCEPTIONDEST_OFFSET(r5)
    oris    r3, r3, OS_CACHED_REGION_PREFIX
    mtlr    r3
    li      r3, 0x30 // MSR_IR | MSR_DR     // turn on memory addressing
    mtmsr   r3
    blr
entry __OSDBINTEND
  /* clang-format on */
}

static asm void __OSDBJump(void){
    /* clang-format off */

    nofralloc
entry __OSDBJUMPSTART
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
    bla     OS_DBJUMPPOINT_ADDR
#else
    bl      __OSDBJUMPDEST
#endif
entry __OSDBJUMPEND
    /* clang-format on */

} 

__OSExceptionHandler
    __OSSetExceptionHandler(__OSException exception, __OSExceptionHandler handler) {
  __OSExceptionHandler oldHandler;
  oldHandler = OSExceptionTable[exception];
  OSExceptionTable[exception] = handler;
  return oldHandler;
}

__OSExceptionHandler __OSGetExceptionHandler(__OSException exception) {
  return OSExceptionTable[exception];
}

static asm void OSExceptionVector(void) {
  /* clang-format off */
    nofralloc

entry __OSEVStart
    // Save r4 into SPRG0
    mtsprg  0, r4

    // Load current context physical address into r4
    lwz     r4, OS_CURRENTCONTEXT_PADDR

    // Save r3 - r5 into the current context
    stw     r3, OS_CONTEXT_R3(r4)
    mfsprg  r3, 0
    stw     r3, OS_CONTEXT_R4(r4)
    stw     r5, OS_CONTEXT_R5(r4)

    lhz     r3, OS_CONTEXT_STATE(r4)
    ori     r3, r3, OS_CONTEXT_STATE_EXC
    sth     r3, OS_CONTEXT_STATE(r4)

    // Save misc registers
    mfcr    r3
    stw     r3, OS_CONTEXT_CR(r4)
    mflr    r3
    stw     r3, OS_CONTEXT_LR(r4)
    mfctr   r3
#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
entry __OSDBJUMPDEST
#endif
    stw     r3, OS_CONTEXT_CTR(r4)
    mfxer   r3
    stw     r3, OS_CONTEXT_XER(r4)
    mfsrr0  r3
    stw     r3, OS_CONTEXT_SRR0(r4)
    mfsrr1  r3
    stw     r3, OS_CONTEXT_SRR1(r4)
    mr      r5, r3

entry __DBVECTOR
    nop

    // Set SRR1[IR|DR] to turn on address
    // translation at the next RFI
    mfmsr   r3
    ori     r3, r3, 0x30
    mtsrr1  r3

    // This lets us change the exception number based on the
    // exception we're installing.
entry __OSEVSetNumber
    addi    r3, 0, 0x0000

    // Load current context virtual address into r4
    lwz     r4, 0xD4

    // Check non-recoverable interrupt
    rlwinm. r5, r5, 0, MSR_RI_BIT, MSR_RI_BIT
    bne     recoverable
    addis   r5, 0,  OSDefaultExceptionHandler@ha
    addi    r5, r5, OSDefaultExceptionHandler@l
    mtsrr0  r5
    rfi
    // NOT REACHED HERE

recoverable:
    // Locate exception handler.
    rlwinm  r5, r3, 2, 22, 29               // r5 contains exception*4
    lwz     r5, OS_EXCEPTIONTABLE_ADDR(r5)
    mtsrr0  r5

    // Final state
    // r3 - exception number
    // r4 - pointer to context
    // r5 - garbage
    // srr0 - exception handler
    // srr1 - address translation enalbed, not yet recoverable

    rfi
    // NOT REACHED HERE
    // The handler will restore state

entry __OSEVEnd
    nop
  /* clang-format on */
}

void __OSUnhandledException(__OSException exception, OSContext* context, u32 dsisr, u32 dar);
asm void OSDefaultExceptionHandler(register __OSException exception, register OSContext* context) {
  /* clang-format off */
    nofralloc
    OS_EXCEPTION_SAVE_GPRS(context)
    mfdsisr r5
    mfdar   r6

    stwu    r1,-8(r1)
    b       __OSUnhandledException
    /* clang-foramt on */
}

void __OSPSInit(void)
{
	PPCMthid2(PPCMfhid2() | 0xA0000000);
	ICFlashInvalidate();
	__sync();
	// clang-format off
    asm
    {
        li      r3, 0
        mtspr   GQR0, r3
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
        mtspr   GQR1, r3
        mtspr   GQR2, r3
        mtspr   GQR3, r3
        mtspr   GQR4, r3
        mtspr   GQR5, r3
        mtspr   GQR6, r3
        mtspr   GQR7, r3
#endif
    }
  // clang-format on
}

#define DI_CONFIG_IDX 0x9
#define DI_CONFIG_CONFIG_MASK 0xFF
u32 __OSGetDIConfig(void) { return (__DIRegs[DI_CONFIG_IDX] & DI_CONFIG_CONFIG_MASK); }

void OSRegisterVersion(const char* id) { OSReport("%s\n", id); }

#if SDK_VERSION >= SDK_VERSION_RVL_2008_01
// Must be defined down here because of data pooling
static const char* AppGameNameForSysMenu = "HAEA";

const char* OSGetAppGamename(void) {
  int i;
  const char* name = (const char*)OSPhysicalToCached(0x3194);

  if (__OSInIPL) {
    name = AppGameNameForSysMenu;
  } else {
    u8 c = name[0];
    if ((s8)c < '0' || ((s8)c > '9' && (s8)c < 'A') || (s8)c > 'Z') {
      name = (const char*)OSPhysicalToCached(0x3180);
    }
  }

  for (i = 0; i < 4; i++) {
    GameNameBuffer[i] = name[i];
  }
  GameNameBuffer[4] = '\0';

  return GameNameBuffer;
}

u8 OSGetAppType(void) {
  if (__OSInIPL) {
    return 0x40;
  }

  return *(u8*)OSPhysicalToCached(0x3184);
}

#endif
