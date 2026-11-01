#include <SDKVersion.h>
#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
u16* __CARDGetFatBlock(CARDControl* card) { return card->currentFat; }
#else
#line 56
u16* __CARDGetFatBlock(CARDControl* card) {
  ASSERT(card->currentFat);
  return card->currentFat;
}
#endif

#line 72
static void WriteCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  u16* fat;
  u16* fatBack;
#else
  u16* fat0;
  u16* fat1;
#endif

  card = &__CARDBlock[chan];
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  if (result >= 0) {
    fat0 = (u16*)((u8*)card->workArea + 0x6000);
    fat1 = (u16*)((u8*)card->workArea + 0x8000);
    ASSERT(card->currentFat);
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  if (result >= 0) {
    fat = (u16*)((u8*)card->workArea + 0x6000);
    fatBack = (u16*)((u8*)card->workArea + 0x8000);
#else
    if (card->currentFat == fat0) {
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
    if (card->currentFat == fat) {
      card->currentFat = fatBack;
      memcpy(fatBack, fat, 0x2000);
#else
      card->currentFat = fat1;
      memcpy(fat1, fat0, 0x2000);
#endif
    } else {
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
      card->currentFat = fat;
      memcpy(fat, fatBack, 0x2000);
#else

      ASSERT(card->currentFat == fat1);
      card->currentFat = fat0;
      memcpy(fat0, fat1, 0x2000);
#endif
    }
  }

  if (card->apiCallback == NULL) {
    __CARDPutControlBlock(card, result);
  }

  callback = card->eraseCallback;
  if (callback) {
    card->eraseCallback = NULL;
    callback(chan, result);
  }
}

static void EraseCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  u32 temp[2]; /* this compiler sucks */
#endif
  u16* fat;
  u32 addr;

  card = &__CARDBlock[chan];
  if (result < 0) {
    goto error;
  }

  fat = __CARDGetFatBlock(card);
  addr = ((u32)fat - (u32)card->workArea) / CARD_SYSTEM_BLOCK_SIZE * card->sectorSize;
  result = __CARDWrite(chan, addr, CARD_SYSTEM_BLOCK_SIZE, fat, WriteCallback);
  if (result < 0) {
    goto error;
  }

  return;

error:
  if (card->apiCallback == NULL) {
    __CARDPutControlBlock(card, result);
  }
  callback = card->eraseCallback;
  if (callback) {
    card->eraseCallback = NULL;
    callback(chan, result);
  }
}

#line 174
s32 __CARDAllocBlock(s32 chan, u32 cBlock, CARDCallback callback) {
  CARDControl* card;
  u16* fat;
  u16 iBlock;
  u16 startBlock;
  u16 prevBlock;
  u16 count;

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  ASSERT(0 < cBlock);
  ASSERT(0 <= chan && chan < 2);
#endif
  card = &__CARDBlock[chan];
  if (!card->attached) {
    return CARD_RESULT_NOCARD;
  }

  fat = __CARDGetFatBlock(card);
  if (fat[3] < cBlock) {
    return CARD_RESULT_INSSPACE;
  }

  fat[3] -= cBlock;
  startBlock = 0xFFFF;
  iBlock = fat[4];
  count = 0;
  while (0 < cBlock) {
    if (card->cBlock - 5 < ++count) {
      return CARD_RESULT_BROKEN;
    }

    iBlock++;
    if (!CARDIsValidBlockNo(card, iBlock)) {
      iBlock = 5;
    }

    if (fat[iBlock] == 0x0000u) {
      if (startBlock == 0xFFFF) {
        startBlock = iBlock;
      } else {
        fat[prevBlock] = iBlock;
      }
      prevBlock = iBlock;
      fat[iBlock] = 0xFFFF;
      --cBlock;
    }
  }
  fat[4] = iBlock;
  card->startBlock = startBlock;

  return __CARDUpdateFatBlock(chan, fat, callback);
}

#line 248
s32 __CARDFreeBlock(s32 chan, u16 nBlock, CARDCallback callback) {
  CARDControl* card;
  u16* fat;
  u16 nextBlock;

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  card = card = &__CARDBlock[chan];
#else
  ASSERT(0 <= chan && chan < 2);
  card = &__CARDBlock[chan];
#endif
  if (!card->attached) {
    return CARD_RESULT_NOCARD;
  }

  fat = __CARDGetFatBlock(card);
  while (nBlock != 0xFFFF) {
    if (!CARDIsValidBlockNo(card, nBlock)) {
      return CARD_RESULT_BROKEN;
    }

    nextBlock = fat[nBlock];
    fat[nBlock] = 0;
    nBlock = nextBlock;
    ++fat[3];
  }

  return __CARDUpdateFatBlock(chan, fat, callback);
}

#line 291
s32 __CARDUpdateFatBlock(s32 chan, u16* fat, CARDCallback callback) {
  CARDControl* card;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  u32 addr;
#endif

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2004_04
  ASSERT(0 <= chan && chan < 2);
#endif
  card = &__CARDBlock[chan];
  ++fat[2];
  __CARDCheckSum(fat + 2, 0x1FFC, fat, fat + 1);
  DCStoreRange(fat, 0x2000);
  card->eraseCallback = callback;

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  return __CARDEraseSector(chan, (((u32)fat - (u32)card->workArea) / 8192u) * card->sectorSize,
                           EraseCallback);
#else
  addr = (((u32)fat - (u32)card->workArea) / 8192u) * card->sectorSize;
  return __CARDEraseSector(chan, addr, EraseCallback);
#endif
}
