#pragma force_active on

#include <revolution/net.h>

static const char *NETRexPPCVersionPrintableString =
    "<< REX-PPC 1.0.0.2 (RevoEX-1.0plus2) REL 070510173628 >>";

const char *NETGetRexPPCVersionPrintable(void) { return NETRexPPCVersionPrintableString; }
