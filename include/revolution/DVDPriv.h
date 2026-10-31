#ifndef _REVOLUTION_DVDPRIV
#define _REVOLUTION_DVDPRIV

#include <revolution/dvd.h>
#include <dolphin/hw_regs.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DVDDriveInfo {
  u16 revisionLevel;
  u16 deviceCode;
  u32 releaseDate;
  u8 padding[24];
} DVDDriveInfo;

typedef struct DVDBB1 {
  u32 appLoaderLength;
  void* appLoaderFunc1;
  void* appLoaderFunc2;
  void* appLoaderFunc3;
} DVDBB1;

typedef struct DVDBB2 {
  u32 bootFilePosition;
  u32 FSTPosition;
  u32 FSTLength;
  u32 FSTMaxLength;
  void* FSTAddress;
  u32 userPosition;
  u32 userLength;

  u32 padding0;
} DVDBB2;

typedef void (*DVDOptionalCommandChecker)(DVDCommandBlock* block, void (*cb)(u32 intType));
typedef void (*DVDLowCallback)(u32 intType);
extern DVDDiskID* DVDGetCurrentDiskID();
DVDLowCallback DVDLowClearCallback();
u32 DVDLowGetLength(void);
BOOL DVDSeekAbsAsyncPrio(DVDCommandBlock* block, s32 offset, DVDCBCallback callback, s32 prio);
void __DVDLowSetWAType(u32 type, u32 location);
DVDCommandBlock* __DVDPopWaitingQueue();

#include <revolution/esp.h>
#include <stdbool.h>

typedef struct diRegVals {
  u32 ImmRegVal;
  u32 CoverRegVal;
  u32 pad[6];
} diRegVals_t;

typedef struct diCommand {
  u8 theCommand;
  u8 pad1[3];
  u32 arg[5];
  u32 pad2[2];
} diCommand_t;

typedef struct DVDVideoReportKey {
  u8 data[32];
} DVDVideoReportKey;

typedef struct DVDGamePartition {
  ESTicket ticket;
  u32 tmdSize;
  ESTitleMeta* tmd;
  u32 certBlobSize;
  void* certBlob;
  u8* h3Hashes;
  u8* encryptedArea;
} DVDGamePartition;

typedef struct DVDPartitionInfo {
  DVDGamePartition* gamePartition;
  u32 type;
} DVDPartitionInfo;

typedef struct DVDGameTOC {
  u32 numGamePartitions;
  DVDPartitionInfo* partitionInfos;
} DVDGameTOC;

typedef struct DVDPartitionParams {
  ESTicket ticket;
  u8 padding0[((sizeof(ESTicket) + 31) & ~31) - sizeof(ESTicket)];
  ESTicketView ticketView;
  u8 padding1[((sizeof(ESTicketView) + 31) & ~31) - sizeof(ESTicketView)];
  u32 numTmdBytes;
  u8 padding2[28];
  ESTitleMeta tmd;
  u8 padding3[((sizeof(ESTitleMeta) + 31) & ~31) - sizeof(ESTitleMeta)];
  u32 numCertBytes;
  u8 padding4[28];
  u8 certificates[4096];
  u32 dataWordOffset;
  u8 padding5[28];
  u8 h3Hash[98304];
} DVDPartitionParams;

bool DVDLowInit(void);
bool DVDLowReadDiskID(DVDDiskID* diskID, DVDLowCallback callback);
bool DVDLowOpenPartition(const u32 wordOffset, const ESTicket* const ticket, const u32 certsSize,
                         const u8* const certs, ESTitleMeta* tmd, DVDLowCallback callback);
bool DVDLowOpenPartitionWithTmdAndTicketView(const u32 wordOffset,
                                             const ESTicketView* const ticketView,
                                             const u32 numTmdBytes, const ESTitleMeta* const tmd,
                                             const u32 numCertBytes, const u8* const certs,
                                             DVDLowCallback callback);
bool DVDLowGetNoDiscBufferSizes(const u32 wordOffset, u32* numTmdBytes, u32* numCertBytes,
                                DVDLowCallback callback);
bool DVDLowGetNoDiscOpenPartitionParams(const u32 wordOffset, ESTicket* ticket, u32* certsSize,
                                        ESTitleMeta* tmd, u32* numTmdBytes, u8* certs,
                                        u32* numCertBytes, u8* h3, DVDLowCallback callback);
bool DVDLowClosePartition(DVDLowCallback callback);
bool DVDLowUnencryptedRead(void* addr, u32 length, u32 offset, DVDLowCallback callback);
bool DVDLowStopMotor(bool eject, bool kill, DVDLowCallback callback);
bool DVDLowInquiry(DVDDriveInfo* info, DVDLowCallback callback);
bool DVDLowRequestError(DVDLowCallback callback);
bool DVDLowSetSpinupFlag(u32 flag);
bool DVDLowReset(DVDLowCallback callback);
bool DVDLowAudioBufferConfig(BOOL enable, u32 size, DVDLowCallback callback);
bool DVDLowReportKey(DVDVideoReportKey* key, u32 format, u32 lba, DVDLowCallback callback);
bool DVDLowSetMaximumRotation(u32 speed, DVDLowCallback callback);
bool DVDLowRead(void* addr, u32 length, u32 offset, DVDLowCallback callback);
bool DVDLowSeek(u32 offset, DVDLowCallback callback);
u32 DVDLowGetCoverRegister(void);
u32 DVDLowGetStatusRegister(void);
bool DVDLowPrepareCoverRegister(DVDLowCallback callback);
bool DVDLowPrepareStatusRegister(DVDLowCallback callback);
u32 DVDLowGetImmBufferReg(void);
bool DVDLowUnmaskStatusInterrupts(void);
bool DVDLowMaskCoverInterrupt(void);
bool DVDLowClearCoverInterrupt(DVDLowCallback callback);

u32 __DVDGetCoverStatus(void);
void __DVDPrepareReset(void);

#ifdef __cplusplus
}
#endif // _REVOLUTION_DVDPRIV

#endif
