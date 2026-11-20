#include "dolphin/types.h"
#include "TRK_MINNOW_DOLPHIN/MetroTRK/Portable/circle_buffer.h"
#include "OdemuExi2/odemuexi/DebuggerDriver.h"

static BOOL gGDEVIsInitialized;
static CircleBuffer gGDEVRecvCB;
static u8 gGDEVRecvBuf[1280];

void MWTRACE(int level, const char* fmt, ...);

int gdev_cc_initialize(volatile u8** inputPendingPtr, __OSInterruptHandler cb)
{
	MWTRACE(1, "CALLING EXI2_Init\n");
	DBInitComm(inputPendingPtr, cb);
	MWTRACE(1, "DONE CALLING EXI2_Init\n");
	CircleBufferInitialize(&gGDEVRecvCB, gGDEVRecvBuf, sizeof(gGDEVRecvBuf));
	return 0;
}

int gdev_cc_shutdown(void) { return 0; }

int gdev_cc_open(void)
{
	if (gGDEVIsInitialized != 0) {
		return -0x2715;
	}
	gGDEVIsInitialized = 1;
	return 0;
}

int gdev_cc_close(void) { return 0; }

int gdev_cc_read(void* data, int size)
{
	u32 err = 0;
	u8 buf[sizeof(gGDEVRecvBuf)];

	if (gGDEVIsInitialized == 0) {
		return -0x2711;
	}

	MWTRACE(1, "Expected packet size : 0x%08x (%ld)\n", size, size);

	while (CBGetBytesAvailableForRead(&gGDEVRecvCB) < (u32)size) {
		int n;
		err = 0;
		n = DBQueryData();
		if (n != 0) {
			err = DBRead(buf, size);
			if (err == 0) {
				CircleBufferWriteBytes(&gGDEVRecvCB, buf, n);
			}
		}
	}

	if (err == 0) {
		CircleBufferReadBytes(&gGDEVRecvCB, data, size);
	} else {
		MWTRACE(8, "cc_read : error reading bytes from EXI2 %ld\n", err);
	}
	return err;
}

int gdev_cc_write(void* data, int size)
{
	int remaining = size;
	u8* cursor = data;
	int n;

	if (gGDEVIsInitialized == 0) {
		MWTRACE(8, "cc not initialized\n");
		return -0x2711;
	}

	MWTRACE(8, "cc_write : Output data 0x%08x %ld bytes\n", data, size);

	while (remaining > 0) {
		MWTRACE(1, "cc_write sending %ld bytes\n", remaining);
		n = DBWrite(cursor, remaining);
		if (n == 0) {
			break;
		}
		cursor += n;
		remaining -= n;
	}
	return 0;
}

int gdev_cc_pre_continue(void)
{
	DBClose();
	return 0;
}

int gdev_cc_post_stop(void)
{
	DBOpen();
	return 0;
}

int gdev_cc_peek(void)
{
	u8 buf[sizeof(gGDEVRecvBuf)];
	int n = DBQueryData();

	if (n <= 0) {
		return 0;
	}
	if (DBRead(buf, n) == 0) {
		CircleBufferWriteBytes(&gGDEVRecvCB, buf, n);
	} else {
		return -0x2719;
	}
	return n;
}

int gdev_cc_initinterrupts(void)
{
	DBInitInterrupts();
	return 0;
}
