#ifndef METROTRK_PORTABLE_MSG_H
#define METROTRK_PORTABLE_MSG_H

#include "trk.h"

#ifdef __cplusplus
extern "C" {
#endif

DSError TRKMessageSend(TRKBuffer* msg);

#ifdef __cplusplus
}
#endif

#endif /* METROTRK_PORTABLE_MSG_H */
