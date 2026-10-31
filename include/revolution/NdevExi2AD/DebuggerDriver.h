#ifndef _REVOLUTION_NDEVEXI2AD_DEBUGGERDRIVER
#define _REVOLUTION_NDEVEXI2AD_DEBUGGERDRIVER

#include <revolution/NdevExi2AD/exi2.h>
#include <revolution/os.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*DBInterruptHandler)(__OSInterrupt interrupt, OSContext *context);

void DBInitComm(u8 **flagOut, DBInterruptHandler handler);
void DBInitInterrupts(void);
u32 DBQueryData(void);
BOOL DBRead(void *dst, u32 size);
BOOL DBWrite(const void *src, u32 size);
void DBOpen(void);
void DBClose(void);

static BOOL __DBReadMailbox(u32 *mailOut) {
  return __DBEXIReadReg(0x34000200, mailOut, sizeof(*mailOut));
}

static BOOL __DBRead(u32 ofs, void *dest, u32 size) {
  return __DBEXIReadRam(((ofs + 0xD10000) * 0x40) & 0x3FFFFF00, dest, size);
}

static BOOL __DBWriteMailbox(u32 mail) {
  return __DBEXIWriteReg(0xB4000100, &mail, sizeof(mail));
}

static BOOL __DBWrite(u32 ofs, const void *src, u32 size) {
  return __DBEXIWriteRam((((ofs + 0xD10000) * 0x40) & 0x3FFFFF00) | 0x80000000, src, size);
}

#ifdef __cplusplus
}
#endif

#endif // _REVOLUTION_NDEVEXI2AD_DEBUGGERDRIVER
