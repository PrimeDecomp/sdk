#include "MetroTRK/Portable/msgbuf.h"
#include "MetroTRK/Portable/mem_TRK.h"
#include "MetroTRK/Portable/nubinit.h"
#include "Os/dolphin/dolphin_trk_log.h"

static TRKBuffer gTRKMsgBufs[3];

static void TRKSetBufferUsed(TRKBuffer* msg, BOOL state) {
  msg->isInUse = state;
}

DSError TRKInitializeMessageBuffers(void) {
  int i;
  for (i = 0; i < 3; i++) {
    TRKSetBufferUsed(&gTRKMsgBufs[i], FALSE);
  }
  return DS_NoError;
}

DSError TRKGetFreeBuffer(int* msgID, TRKBuffer** outMsg) {
  DSError error = DS_NoMessageBufferAvailable;
  int i;
  TRKBuffer* buf;

  *outMsg = NULL;

  for (i = 0; i < 3; i++) {
    buf = TRKGetBuffer(i);
    if (!buf->isInUse) {
      TRKResetBuffer(buf, TRUE);
      TRKSetBufferUsed(buf, TRUE);
      error = DS_NoError;
      *outMsg = buf;
      *msgID = i;
      i = 3;
    }
  }

  if (error == DS_NoMessageBufferAvailable) {
    OSReport("MetroTRK - ERROR : No buffer available\n");
  }
  return error;
}

void* TRKGetBuffer(int idx) {
  TRKBuffer* buf = NULL;
  if (idx >= 0 && idx < 3) {
    buf = &gTRKMsgBufs[idx];
  }
  return buf;
}

void TRKReleaseBuffer(int idx) {
  if (idx != -1 && idx >= 0 && idx < 3) {
    TRKSetBufferUsed(&gTRKMsgBufs[idx], FALSE);
  }
}

void TRKResetBuffer(TRKBuffer* msg, BOOL keepData) {
  msg->length = 0;
  msg->position = 0;

  if (!keepData) {
    TRK_fill_mem(msg->data, 0, TRKMSGBUF_SIZE);
  }
}

DSError TRKSetBufferPosition(TRKBuffer* msg, u32 pos) {
  DSError error = DS_NoError;

  if (pos > TRKMSGBUF_SIZE) {
    error = DS_MessageBufferOverflow;
  } else {
    msg->position = pos;
    if (pos > msg->length) {
      msg->length = pos;
    }
  }
  return error;
}

DSError TRKAppendBuffer(TRKBuffer* msg, const void* data, size_t length) {
  DSError error = DS_NoError;
  u32 bytesLeft;

  if (length == 0) {
    return DS_NoError;
  }

  bytesLeft = TRKMSGBUF_SIZE - msg->position;
  if (bytesLeft < length) {
    error = DS_MessageBufferOverflow;
    length = bytesLeft;
  }

  if (length == 1) {
    msg->data[msg->position] = ((u8*)data)[0];
  } else {
    TRK_memcpy(msg->data + msg->position, data, length);
  }

  msg->position += length;
  msg->length = msg->position;
  return error;
}

DSError TRKReadBuffer(TRKBuffer* msg, void* data, size_t length) {
  DSError error = DS_NoError;
  unsigned int bytesLeft;

  if (length == 0) {
    return DS_NoError;
  }

  bytesLeft = msg->length - msg->position;
  if (length > bytesLeft) {
    error = DS_MessageBufferReadError;
    length = bytesLeft;
  }

  TRK_memcpy(data, msg->data + msg->position, length);
  msg->position += length;
  return error;
}

static DSError TRKAppendBuffer1_ui8(TRKBuffer* buffer, const u8 data) {
  if (buffer->position >= TRKMSGBUF_SIZE) {
    return DS_MessageBufferOverflow;
  }

  buffer->data[buffer->position++] = data;
  buffer->length++;
  return DS_NoError;
}

DSError TRKAppendBuffer1_ui32(TRKBuffer* buffer, const u32 data) {
  u8* bigEndianData;
  u8* byteData;
  u8 swapBuffer[sizeof(data)];

  if (gTRKBigEndian) {
    bigEndianData = (u8*)&data;
  } else {
    byteData = (u8*)&data;
    bigEndianData = swapBuffer;

    bigEndianData[0] = byteData[3];
    bigEndianData[1] = byteData[2];
    bigEndianData[2] = byteData[1];
    bigEndianData[3] = byteData[0];
  }

  return TRKAppendBuffer(buffer, (const void*)bigEndianData, sizeof(data));
}

DSError TRKAppendBuffer1_ui64(TRKBuffer* buffer, const u64 data) {
  u8* bigEndianData;
  u8* byteData;
  u8 swapBuffer[sizeof(data)];

  if (gTRKBigEndian) {
    bigEndianData = (u8*)&data;
  } else {
    byteData = (u8*)&data;
    bigEndianData = swapBuffer;

    bigEndianData[0] = byteData[7];
    bigEndianData[1] = byteData[6];
    bigEndianData[2] = byteData[5];
    bigEndianData[3] = byteData[4];
    bigEndianData[4] = byteData[3];
    bigEndianData[5] = byteData[2];
    bigEndianData[6] = byteData[1];
    bigEndianData[7] = byteData[0];
  }

  return TRKAppendBuffer(buffer, (const void*)bigEndianData, sizeof(data));
}

DSError TRKAppendBuffer_ui8(TRKBuffer* buffer, const u8* data, int count) {
  DSError err;
  int i;

  for (i = 0, err = DS_NoError; err == DS_NoError && i < count; i++) {
    err = TRKAppendBuffer1_ui8(buffer, data[i]);
  }
  return err;
}

DSError TRKAppendBuffer_ui32(TRKBuffer* buffer, const u32* data, int count) {
  DSError err;
  int i;

  for (i = 0, err = DS_NoError; err == DS_NoError && i < count; i++) {
    err = TRKAppendBuffer1_ui32(buffer, data[i]);
  }
  return err;
}

DSError TRKReadBuffer1_ui8(TRKBuffer* buffer, u8* data) {
  return TRKReadBuffer(buffer, (void*)data, 1);
}

static DSError TRKReadBuffer1_ui32(TRKBuffer* buffer, u32* data) {
  DSError err;
  u8* bigEndianData;
  u8* byteData;
  u8 swapBuffer[sizeof(*data)];

  if (gTRKBigEndian) {
    bigEndianData = (u8*)data;
  } else {
    bigEndianData = swapBuffer;
  }

  err = TRKReadBuffer(buffer, (void*)bigEndianData, sizeof(*data));

  if (!gTRKBigEndian && err == DS_NoError) {
    byteData = (u8*)data;

    byteData[0] = bigEndianData[3];
    byteData[1] = bigEndianData[2];
    byteData[2] = bigEndianData[1];
    byteData[3] = bigEndianData[0];
  }
  return err;
}

DSError TRKReadBuffer1_ui64(TRKBuffer* buffer, u64* data) {
  DSError err;
  u8* bigEndianData;
  u8* byteData;
  u8 swapBuffer[sizeof(*data)];

  if (gTRKBigEndian) {
    bigEndianData = (u8*)data;
  } else {
    bigEndianData = swapBuffer;
  }

  err = TRKReadBuffer(buffer, (void*)bigEndianData, sizeof(*data));

  if (!gTRKBigEndian && err == 0) {
    byteData = (u8*)data;

    byteData[0] = bigEndianData[7];
    byteData[1] = bigEndianData[6];
    byteData[2] = bigEndianData[5];
    byteData[3] = bigEndianData[4];
    byteData[4] = bigEndianData[3];
    byteData[5] = bigEndianData[2];
    byteData[6] = bigEndianData[1];
    byteData[7] = bigEndianData[0];
  }
  return err;
}

DSError TRKReadBuffer_ui8(TRKBuffer* buffer, u8* data, int count) {
  DSError err;
  int i;

  for (i = 0, err = DS_NoError; err == DS_NoError && i < count; i++) {
    err = TRKReadBuffer1_ui8(buffer, &data[i]);
  }
  return err;
}

DSError TRKReadBuffer_ui32(TRKBuffer* buffer, u32* data, int count) {
  DSError err;
  int i;

  for (i = 0, err = DS_NoError; err == DS_NoError && i < count; i++) {
    err = TRKReadBuffer1_ui32(buffer, &data[i]);
  }
  return err;
}
