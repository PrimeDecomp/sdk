#include <OdemuExi2/odemuexi/DebuggerDriver.h>
#include <dolphin/exi.h>
#include <dolphin/hw_regs.h>

// Guessed name
typedef void (*DBGInterruptCallback)(s32 interrupt, OSContext* context);

static __OSInterruptHandler sCommCallback;
static DBGInterruptCallback sIntCallback;
static u32 sLastMailbox;
static s32 sReadLength;
static volatile u8* sCommInputFlag;
static volatile u8 sRxPending;
static u8 sSendCount = 0x80;

void DBGInterrupt(__OSInterrupt interrupt, OSContext* context);
void DBGCommInterrupt(s32 interrupt, OSContext* context);
static inline BOOL DBGReadStatus(u32* status);
BOOL DBGWriteBlock(u32 addr, const void* buf, s32 len);
BOOL DBGReadBlock(u32 addr, void* buf, s32 len);
static BOOL DBGReadMailbox(u32* mailbox);
static BOOL DBGEXIImm(void* buf, s32 len, u32 rw);

// Guessed name
static inline BOOL DBGEXISelect(u32 speed) {
    u32 csr = __EXIRegs[10];
    csr &= 0x405;
    csr |= 0x80 | (speed << 4);
    __EXIRegs[10] = csr;
    return TRUE;
}

// Guessed name
static inline BOOL DBGEXIDeselect(void) {
    __EXIRegs[10] &= 0x405;
    return TRUE;
}

// Guessed name
static inline BOOL DBGEXISync(void) {
    while (__EXIRegs[13] & 1) {}
    return TRUE;
}

// Guessed name
static inline BOOL DBGWriteMailbox(u32 mailbox) {
    BOOL error = FALSE;
    u32 word;

    if (!DBGEXISelect(4)) {
        return FALSE;
    }
    word = (mailbox & 0x1FFFFFFF) | 0xC0000000;
    error |= !DBGEXIImm(&word, sizeof(word), EXI_WRITE);
    error |= !DBGEXISync();
    error |= !DBGEXIDeselect();
    return !error;
}

// Guessed name
static inline BOOL DBGReadCommand(u32 command, u32* result) {
    BOOL error = FALSE;
    u32 word;

    if (!DBGEXISelect(4)) {
        return FALSE;
    }
    word = command;
    error |= !DBGEXIImm(&word, 2, EXI_WRITE);
    error |= !DBGEXISync();
    error |= !DBGEXIImm(result, sizeof(*result), EXI_READ);
    error |= !DBGEXISync();
    error |= !DBGEXIDeselect();
    return !error;
}

void DBClose(void) {}

void DBOpen(void) {}

int DBWrite(const void* buf, u32 len) {
    BOOL enabled;
    u32 status;
    u32 header;
    u32 offset;
    enabled = OSDisableInterrupts();

    do {
        DBGReadCommand(0x40000000, &status);
    } while (status & 2);

    sSendCount++;
    offset = (sSendCount & 1) ? 0x1000 : 0;
    while (!DBGWriteBlock(offset | 0x1C000, buf, (len + 3) & ~3)) {}

    do {
        DBGReadCommand(0x40000000, &status);
    } while (status & 2);

    header = ((u32)sSendCount << 16) | 0x1F000000 | len;
    while (!DBGWriteMailbox(header)) {}

    do {
        while (!DBGReadCommand(0x40000000, &status)) {}
    } while (status & 2);

    OSRestoreInterrupts(enabled);
    return 0;
}

int DBRead(void* buf, u32 len) {
    BOOL enabled;
    u32 offset;

    enabled = OSDisableInterrupts();

    offset = (sLastMailbox & 0x10000) ? 0x1000 : 0;
    DBGReadBlock(offset + 0x1E000, buf, (len + 3) & ~3);

    sReadLength = 0;
    sRxPending = 0;

    OSRestoreInterrupts(enabled);
    return 0;
}

u32 DBQueryData(void) {
    BOOL enabled;
    u32 status;

    sRxPending = 0;

    if (sReadLength == 0) {
        enabled = OSDisableInterrupts();

        DBGReadStatus(&status);
        if (status & 1) {
            DBGReadMailbox(&status);
            status &= 0x1FFFFFFF;

            if ((status & 0x1F000000) == 0x1F000000) {
                sLastMailbox = status;
                sReadLength = status & 0x7FFF;
                sRxPending = 1;
            }
        }

        OSRestoreInterrupts(enabled);
    }

    return sReadLength;
}

void DBInitInterrupts(void) {
    __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_2);
    __OSMaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
    sIntCallback = DBGCommInterrupt;
    __OSSetInterruptHandler(__OS_INTERRUPT_PI_DEBUG, DBGInterrupt);
    __OSUnmaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
}

void DBInitComm(volatile u8** inputFlagPtr, __OSInterruptHandler mtrCallback) {
    BOOL enabled;

    enabled = OSDisableInterrupts();
    sCommInputFlag = &sRxPending;
    *inputFlagPtr = sCommInputFlag;
    sCommCallback = mtrCallback;

    __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_2);
    __EXIRegs[10] = 0;

    OSRestoreInterrupts(enabled);
}

// Guessed name
void DBGInterrupt(__OSInterrupt interrupt, OSContext* context) {
    __PIRegs[0] = 1 << 12;
    if (sIntCallback) {
        sIntCallback(interrupt, context);
    }
}

// Guessed name
void DBGCommInterrupt(s32 interrupt, OSContext* context) {
    sRxPending = 1;
    if (sCommCallback) {
        sCommCallback(0, context);
    }
}

static inline BOOL DBGReadStatus(u32* status) {
    return DBGReadCommand(0x40000000, status);
}

// Guessed name
BOOL DBGWriteBlock(u32 addr, const void* buf, s32 len) {
    BOOL err = FALSE;
    const u32* src = buf;
    u32 cmd;
    u32 word;

    if (!DBGEXISelect(4)) {
        return FALSE;
    }
    cmd = ((addr << 8) & 0x01FFFC00) | 0xA0000000;
    err |= !DBGEXIImm(&cmd, sizeof(cmd), EXI_WRITE);
    err |= !DBGEXISync();
    while (len != 0) {
        word = *src++;
        err |= !DBGEXIImm(&word, sizeof(word), EXI_WRITE);
        err |= !DBGEXISync();
        len -= 4;
        if (len < 0) {
            len = 0;
        }
    }
    err |= !DBGEXIDeselect();
    return !err;
}

// Guessed name
BOOL DBGReadBlock(u32 addr, void* buf, s32 len) {
    BOOL err = FALSE;
    u32* dst = buf;
    u32 cmd;
    u32 word;

    if (!DBGEXISelect(4)) {
        return FALSE;
    }
    cmd = ((addr << 8) & 0x01FFFC00) | 0x20000000;
    err |= !DBGEXIImm(&cmd, sizeof(cmd), EXI_WRITE);
    err |= !DBGEXISync();
    while (len != 0) {
        err |= !DBGEXIImm(&word, sizeof(word), EXI_READ);
        err |= !DBGEXISync();
        *dst++ = word;
        len -= 4;
        if (len < 0) {
            len = 0;
        }
    }
    err |= !DBGEXIDeselect();
    return !err;
}

static BOOL DBGReadMailbox(u32* mailbox) {
    return DBGReadCommand(0x60000000, mailbox);
}

static BOOL DBGEXIImm(void* buf, s32 len, u32 rw) {
    u32 writeWord;
    u32 readWord;
    int i;

    if (rw) {
        writeWord = 0;
        for (i = 0; i < len; ++i) {
            const u8* byte = (const u8*)buf + i;
            writeWord |= (u32)*byte << ((3 - i) << 3);
        }
        __EXIRegs[14] = writeWord;
    }

    __EXIRegs[13] = 1 | (rw << 2) | ((len - 1U) << 4);
    DBGEXISync();

    if (!rw) {
        u8* bytes = buf;
        readWord = __EXIRegs[14];
        for (i = 0; i < len; ++i) {
            *bytes++ = (u8)(readWord >> ((3 - i) << 3));
        }
    }

    return TRUE;
}
