#include "MetroTRK/Portable/notify.h"
#include "MetroTRK/Portable/msgbuf.h"
#include "MetroTRK/Portable/support.h"
#include "ppc/Generic/targimpl.h"
#include "trk.h"

DSError TRKDoNotifyStopped(MessageCommandID cmd) {
    int reqIdx;
    int bufIdx;
    TRKBuffer* msg;
    DSError err;

    err = TRKGetFreeBuffer(&bufIdx, &msg);
    if (err == DS_NoError) {
        if (cmd == DSMSG_NotifyStopped) {
            TRKTargetAddStopInfo(msg);
        } else {
            TRKTargetAddExceptionInfo(msg);
        }
        err = TRKRequestSend(msg, &reqIdx);
        if (err == DS_NoError) {
            TRKReleaseBuffer(reqIdx);
        }
        TRKReleaseBuffer(bufIdx);
    }
    return err;
}
