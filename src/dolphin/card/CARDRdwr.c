#include <dolphin/card.h>
#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#include <dolphin/CARDPriv.h>

static void BlockReadCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;

  card = &__CARDBlock[chan];
  if (result < 0) {
    goto error;
  }

  card->xferred += CARD_SEG_SIZE;

  card->addr += CARD_SEG_SIZE;
  (u8*)card->buffer += CARD_SEG_SIZE;
  if (--card->repeat <= 0) {
    goto error;
  }

  result = __CARDReadSegment(chan, BlockReadCallback);
  if (result < 0) {
    goto error;
  }
  return;

error:
  if (card->apiCallback == 0) {
    __CARDPutControlBlock(card, result);
  }
  callback = card->xferCallback;
  if (callback) {
    card->xferCallback = 0;
    callback(chan, result);
  }
}

#ifndef PRIME1
#line 88
#endif
s32 __CARDRead(s32 chan, u32 addr, s32 length, void* dst, CARDCallback callback) {
  CARDControl* card;
#ifndef PRIME1

  ASSERT(0 < length && length % CARD_SEG_SIZE == 0);
  ASSERT(0 <= chan && chan < 2);

#endif
  card = &__CARDBlock[chan];
  if (!card->attached) {
    return CARD_RESULT_NOCARD;
  }

  card->xferCallback = callback;
  card->repeat = (int)(length / CARD_SEG_SIZE);
  card->addr = addr;
  card->buffer = dst;

  return __CARDReadSegment(chan, BlockReadCallback);
}

static void BlockWriteCallback(s32 chan, s32 result) {
  CARDControl* card;
  CARDCallback callback;

  card = &__CARDBlock[chan];
  if (result < 0) {
    goto error;
  }

#ifdef PRIME1
  card->xferred += CARD_PAGE_SIZE;
#else
  card->xferred += card->pageSize;
#endif

#ifdef PRIME1
  card->addr += CARD_PAGE_SIZE;
  (u8*)card->buffer += CARD_PAGE_SIZE;
#else
  card->addr += card->pageSize;
  (u8*)card->buffer += card->pageSize;
#endif
  if (--card->repeat <= 0) {
    goto error;
  }

  result = __CARDWritePage(chan, BlockWriteCallback);
  if (result < 0) {
    goto error;
  }
  return;

error:
  if (card->apiCallback == 0) {
    __CARDPutControlBlock(card, result);
  }
  callback = card->xferCallback;
  if (callback) {
    card->xferCallback = 0;
    callback(chan, result);
  }
}

#ifndef PRIME1
#line 149
#endif
s32 __CARDWrite(s32 chan, u32 addr, s32 length, void* dst, CARDCallback callback) {
  CARDControl* card;
#ifndef PRIME1

#endif
  card = &__CARDBlock[chan];
#ifndef PRIME1
  ASSERT(0 < length && length % card->pageSize == 0);
  ASSERT(0 <= chan && chan < 2);
#endif
  if (!card->attached) {
    return CARD_RESULT_NOCARD;
  }

  card->xferCallback = callback;
#ifdef PRIME1
  card->repeat = (int)(length / CARD_PAGE_SIZE);
#else
  card->repeat = (int)(length / card->pageSize);
#endif
  card->addr = addr;
  card->buffer = dst;

  return __CARDWritePage(chan, BlockWriteCallback);
}

#ifndef PRIME1
#line 182
s32 CARDGetXferredBytes(s32 chan) {
  ASSERT(0 <= chan && chan < 2);
  return __CARDBlock[chan].xferred;
}

#endif