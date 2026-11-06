#include <SDKVersion.h>

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
#include <dolphin/types.h>
#include <TRK_MINNOW_DOLPHIN/TrkInit.h>
#pragma force_active on
BOOL gTRKBigEndian = FALSE;
#pragma force_active reset

#if 0
DSError TRKInitializeNub(void)
{
  DSError result;
  DSError resultTemp;

  result = TRKInitializeEndian();

  if (result == kNoError) {
    usr_put_initialize();
  }

  if (result == kNoError) {
    result = TRKInitializeEventQueue();
  }

  if (result == kNoError) {
    result = TRKInitializeMessageBuffers();
  }

  // v0.1 only
  if (result == kNoError) {
    result = TRKInitializeDispatcher();
  }

  if (result == kNoError) {
    // 0xE100 only on v0.1
    resultTemp =
        TRKInitializeIntDrivenUART(0xE100, 1, 0, &gTRKInputPendingPtr);
    TRKTargetSetInputPendingPtr(gTRKInputPendingPtr);
    if (resultTemp != kNoError) {
      result = resultTemp;
    }
  }

  if (result == kNoError) {
    result = TRKInitializeSerialHandler();
  }

  if (result == kNoError) {
    result = TRKInitializeTarget();
  }

  return result;
}

DSError TRKTerminateNub(void)
{
  TRKTerminateSerialHandler();
  return kNoError;
}

void TRKNubWelcome(void)
{
  TRK_board_display("MetroTRK for Dolphin v0.8");
}
#endif

BOOL TRKInitializeEndian(void)
{
  u8 bendian[4];
  BOOL result = FALSE;

  gTRKBigEndian = TRUE;

  bendian[0] = 0x12;
  bendian[1] = 0x34;
  bendian[2] = 0x56;
  bendian[3] = 0x78;

  if (*(u32*) bendian == 0x12345678) {
    gTRKBigEndian = TRUE;
  } else if (*(u32*) bendian == 0x78563412) {
    gTRKBigEndian = FALSE;
  } else {
    result = TRUE;
  }

  return result;
}
#else
#include "TRK_MINNOW_DOLPHIN/MetroTRK/Portable/nubinit.h"
#include "TRK_MINNOW_DOLPHIN/MetroTRK/Portable/nubevent.h"
#include "TRK_MINNOW_DOLPHIN/MetroTRK/Portable/msgbuf.h"
#include "TRK_MINNOW_DOLPHIN/MetroTRK/Portable/serpoll.h"
#include "TRK_MINNOW_DOLPHIN/MetroTRK/Portable/dispatch.h"
#include "TRK_MINNOW_DOLPHIN/MetroTRK/Portable/serpoll.h"
#include "TRK_MINNOW_DOLPHIN/Os/dolphin/dolphin_trk.h"
#include "TRK_MINNOW_DOLPHIN/Os/dolphin/dolphin_trk_glue.h"
#include "TRK_MINNOW_DOLPHIN/Os/dolphin/usr_put.h"
#include "TRK_MINNOW_DOLPHIN/ppc/Generic/targimpl.h"

BOOL gTRKBigEndian;

void MWTRACE(int level, const char* fmt, ...);
void InitializeProgramEndTrap(void);

DSError TRKInitializeNub(void)
{
	DSError ret;
	DSError uartErr;

	{
		u8 bendian[4];
		ret           = DS_NoError;
		gTRKBigEndian = TRUE;

		bendian[0] = 0x12;
		bendian[1] = 0x34;
		bendian[2] = 0x56;
		bendian[3] = 0x78;

		if (*(u32*)bendian == 0x12345678) {
			gTRKBigEndian = TRUE;
		} else if (*(u32*)bendian == 0x78563412) {
			gTRKBigEndian = FALSE;
		} else {
			ret = TRUE;
		}
	}

	MWTRACE(1, "Initialize NUB\n");

	if (ret == DS_NoError)
		usr_put_initialize();
	if (ret == DS_NoError)
		ret = TRKInitializeEventQueue();
	if (ret == DS_NoError)
		ret = TRKInitializeMessageBuffers();
	if (ret == DS_NoError)
		ret = TRKInitializeDispatcher();

	InitializeProgramEndTrap();

	if (ret == DS_NoError)
		ret = TRKInitializeSerialHandler();
	if (ret == DS_NoError)
		ret = TRKInitializeTarget();

	if (ret == DS_NoError) {
		uartErr = TRKInitializeIntDrivenUART(0x0000e100, 1, 0,
		                                     (volatile u8**)&gTRKInputPendingPtr);
		TRKTargetSetInputPendingPtr(gTRKInputPendingPtr);
		if (uartErr != DS_NoError) {
			ret = uartErr;
		}
	}

	return ret;
}

DSError TRKTerminateNub(void)
{
	TRKTerminateSerialHandler();
	return DS_NoError;
}

void TRKNubWelcome(void)
{
	TRK_board_display("MetroTRK for GAMECUBE v2.6");
	return;
}
#endif
