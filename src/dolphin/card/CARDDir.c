#include <SDKVersion.h>
#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
CARDDir* __CARDGetDirBlock(CARDControl* card) { return card->currentDir; }
#else
#line 53
CARDDir* __CARDGetDirBlock(CARDControl* card) {
  ASSERT(card->currentDir);
  return card->currentDir;
}
#endif

#line 69
static void WriteCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
  CARDDir* dir0;
  CARDDir* dir1;
#endif

  card = &__CARDBlock[chan];
  if (0 <= result) {
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    CARDDir* dir0 = (CARDDir*)((u8*)card->workArea + 0x2000);
    CARDDir* dir1 = (CARDDir*)((u8*)card->workArea + 0x4000);
#else
    dir0 = (CARDDir*)((u8*)card->workArea + 0x2000);
    dir1 = (CARDDir*)((u8*)card->workArea + 0x4000);
    ASSERT(card->currentDir);
#endif

    if (card->currentDir == dir0) {
      card->currentDir = dir1;
      memcpy(dir1, dir0, 0x2000);
    } else {
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
    
      ASSERT(card->currentDir == dir1);
#endif
      card->currentDir = dir0;
      memcpy(dir0, dir1, 0x2000);
    }
  }

error:
  if (card->apiCallback == 0) {
    __CARDPutControlBlock(card, result);
  }
  callback = card->eraseCallback;
  if (callback) {
    card->eraseCallback = 0;
    callback(chan, result);
  }
}

static void EraseCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
  CARDDir* dir;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  u32 tmp[2];
#endif
  u32 addr;

  card = &__CARDBlock[chan];
  if (result < 0) {
    goto error;
  }

  dir = __CARDGetDirBlock(card);
  addr = ((u32)dir - (u32)card->workArea) / 0x2000 * card->sectorSize;
  result = __CARDWrite(chan, addr, 0x2000, dir, WriteCallback);
  if (result < 0) {
    goto error;
  }

  return;

error:
  if (card->apiCallback == 0) {
    __CARDPutControlBlock(card, result);
  }
  callback = card->eraseCallback;
  if (callback) {
    card->eraseCallback = 0;
    callback(chan, result);
  }
}

#line 167
s32 __CARDUpdateDir(s32 chan, CARDCallback callback) {
  CARDControl* card;
  CARDDirCheck* check;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  u32 tmp[2];
#endif
  u32 addr;
  CARDDir* dir;

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
  ASSERT(0 <= chan && chan < 2);
#endif
  card = &__CARDBlock[chan];
  if (!card->attached) {
    return CARD_RESULT_NOCARD;
  }

  dir = __CARDGetDirBlock(card);
  check = __CARDGetDirCheck(dir);
  ++check->checkCode;
  __CARDCheckSum(dir, 0x2000 - sizeof(u32), &check->checkSum, &check->checkSumInv);
  DCStoreRange(dir, 0x2000);

  card->eraseCallback = callback;
  addr = ((u32)dir - (u32)card->workArea) / 0x2000 * card->sectorSize;
  return __CARDEraseSector(chan, addr, EraseCallback);
}
