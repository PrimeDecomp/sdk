#include "MetroTRK/Portable/msg.h"
#include "Os/dolphin/dolphin_trk_log.h"

static u16 gTRKMsgSequence;

DSError TRKMessageSend(TRKBuffer* msg) {
  u16 seq = gTRKMsgSequence;
  int err;

  if (seq == 0) {
    seq = 1;
  }
  *(u16*)&msg->data[6] = seq;
  gTRKMsgSequence = ++seq;

  err = TRKWriteUARTN(msg->data, msg->length);
  if (err != 0) {
    OSReport("MetroTRK - TRK_WriteUARTN returned %ld\n", err);
  }
  return DS_NoError;
}
