#include <dolphin/db.h>
#include <dolphin/exi.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>

typedef void (*DBGCallback)(s32);

static u32 sLastMailbox;
static u32 sReadLength;
static u32 sCommInputFlag;
static u8 sRxPending;
static u8 sSendCount = 0;
static DBGCallback sCommCallback;
static DBGCallback sIntCallback;

#define DBG_CSR_SELECT()   (__EXIRegs[10] = ((__EXIRegs[10] & 0x405) | 0xC0))
#define DBG_CSR_RELEASE()  (__EXIRegs[10] &= 0x405)

void DBGInterrupt(__OSInterrupt interrupt, OSContext* context);
void DBGCommInterrupt(s32 interrupt);
static BOOL DBGReadStatus(u32* status);
BOOL DBGWriteBlock(u32 addr, void* buf, s32 len);
BOOL DBGReadBlock(u32 addr, void* buf, s32 len);
static BOOL DBGReadMailbox(u32* mailbox);
static BOOL DBGEXIImm(void* buf, s32 len, s32 rw);

void DBClose(void) {}

void DBOpen(void) {}

BOOL DBWrite(void* buf, s32 len) {
    BOOL enabled;
    u32 cmd;
    u32 status;
    u32 header;
    u32 addr;
    BOOL ok;

    enabled = OSDisableInterrupts();

    do {
        DBGReadStatus(&status);
    } while (status & 2);

    sSendCount++;
    addr = 0x1C000;
    if (sSendCount & 1) {
        addr |= 0x1000;
    }

    do {
        ok = DBGWriteBlock(addr, buf, (len + 3) & ~3);
    } while (!ok);

    do {
        DBGReadStatus(&status);
    } while (status & 2);

    header = 0xC0000000 | ((((u32)sSendCount << 16) | 0x1F000000 | (u32)len) & 0x1FFFFFFF);
    do {
        DBG_CSR_SELECT();
        ok = DBGEXIImm(&header, 4, 1);
        DBG_CSR_RELEASE();
    } while (!ok);

    do {
        ok = DBGReadStatus(&status);
    } while (!ok || (status & 2));

    OSRestoreInterrupts(enabled);
    return 0;
}

BOOL DBRead(void* buf, s32 len) {
    BOOL enabled;
    u32 addr;

    enabled = OSDisableInterrupts();

    addr = 0x1E000;
    if (sLastMailbox & 0x10000) {
        addr |= 0x1000;
    }
    DBGReadBlock(addr, buf, (len + 3) & ~3);

    sReadLength = 0;
    sRxPending = 0;

    OSRestoreInterrupts(enabled);
    return 0;
}

u32 DBQueryData(void) {
    BOOL enabled;
    u32 status;
    u32 mailbox;

    sRxPending = 0;

    if (sReadLength == 0) {
        enabled = OSDisableInterrupts();

        DBGReadStatus(&status);
        if (status & 1) {
            DBGReadMailbox(&mailbox);
            mailbox &= 0x1FFFFFFF;

            if ((mailbox & 0x1F000000) == 0x1F000000) {
                sLastMailbox = mailbox;
                sReadLength = mailbox & 0x7FFF;
                sRxPending = 1;
            }
        }

        OSRestoreInterrupts(enabled);
    }

    return sReadLength;
}

void DBInitInterrupts(void) {
    __OSMaskInterrupts(0x18000);
    __OSMaskInterrupts(0x40);
    sIntCallback = DBGCommInterrupt;
    __OSSetInterruptHandler(0x19, DBGInterrupt);
    __OSUnmaskInterrupts(0x40);
}

void DBInitComm(int* inputFlagPtr, int* mtrCallback) {
    BOOL enabled;

    enabled = OSDisableInterrupts();

    sRxPending = 0;
    sCommInputFlag = 0;
    *inputFlagPtr = sCommInputFlag;
    sCommCallback = (DBGCallback)mtrCallback;

    __OSMaskInterrupts(0x18000);
    __EXIRegs[10] = 0;

    OSRestoreInterrupts(enabled);
}

// Guessed name: debugger interrupt entry; acknowledges the PI debugger bit
// and forwards the interrupt number to the registered interrupt callback.
void DBGInterrupt(__OSInterrupt interrupt, OSContext* context) {
    __PIRegs[0] = 0x1000;
    if (sIntCallback) {
        sIntCallback((s16)interrupt);
    }
}

// Guessed name: sets the comm-ready flag and forwards to the MW callback.
void DBGCommInterrupt(s32 interrupt) {
    sRxPending = 1;
    if (sCommCallback) {
        sCommCallback(0);
    }
}

static BOOL DBGReadStatus(u32* status) {
    u32 cmd = 0x40000000;
    BOOL cmdErr;
    BOOL readErr;

    DBG_CSR_SELECT();
    cmdErr = !DBGEXIImm(&cmd, 2, 1);
    readErr = !DBGEXIImm(status, 4, 0);
    DBG_CSR_RELEASE();

    return !(cmdErr | readErr);
}

// Guessed name: writes len bytes to debugger memory at addr via a write command.
BOOL DBGWriteBlock(u32 addr, void* buf, s32 len) {
    u32 cmd = ((addr << 8) & 0x01FFFC00) | 0xA0000000;
    u32 word;
    u32* src = buf;
    BOOL err;

    DBG_CSR_SELECT();
    err = !DBGEXIImm(&cmd, 4, 1);
    for (; len > 0; len -= 4) {
        word = *src++;
        err |= !DBGEXIImm(&word, 4, 1);
    }
    DBG_CSR_RELEASE();

    return !err;
}

// Guessed name: reads len bytes from debugger memory at addr via a read command.
BOOL DBGReadBlock(u32 addr, void* buf, s32 len) {
    u32 cmd = ((addr << 8) & 0x01FFFC00) | 0x20000000;
    u32 word;
    u32* dst = buf;
    BOOL err;

    DBG_CSR_SELECT();
    err = !DBGEXIImm(&cmd, 4, 1);
    for (; len > 0; len -= 4) {
        err |= !DBGEXIImm(&word, 4, 0);
        *dst++ = word;
    }
    DBG_CSR_RELEASE();

    return !err;
}

static BOOL DBGReadMailbox(u32* mailbox) {
    u32 cmd = 0x60000000;
    BOOL cmdErr;
    BOOL readErr;

    DBG_CSR_SELECT();
    cmdErr = !DBGEXIImm(&cmd, 2, 1);
    readErr = !DBGEXIImm(mailbox, 4, 0);
    DBG_CSR_RELEASE();

    return !(cmdErr | readErr);
}

// Immediate EXI transfer on the debugger channel. Packs a word big-endian
// for writes and unpacks one for reads. Callers transfer at most one word.
static BOOL DBGEXIImm(void* buf, s32 len, s32 rw) {
    u8* p = buf;
    u32 word = 0;
    s32 i;

    if (rw) {
        for (i = 0; i < len && i < 4; i++) {
            word |= (u32)p[i] << ((3 - i) * 8);
        }
        __EXIRegs[14] = word;
    }

    __EXIRegs[13] = ((u32)(len - 1) << 4) | ((u32)rw << 2) | 1;
    while (__EXIRegs[13] & 1) {}

    if (!rw) {
        word = __EXIRegs[14];
        for (i = 0; i < len && i < 4; i++) {
            p[i] = (u8)(word >> ((3 - i) * 8));
        }
    }

    return TRUE;
}
