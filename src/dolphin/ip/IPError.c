#include <dolphin/ip.h>
#include <dolphin/ip/IPArp.h>
#include <dolphin/private/ip.h>

#define ROUTE_TABLE_SIZE 32

#define ROUTE_GATEWAY  0x1
#define ROUTE_DYNAMIC  0x2
#define ROUTE_MODIFIED 0x4

static BOOL Initialized; // size: 0x4, address: 0x0
static IFQueue Up; // size: 0x8, address: 0x4
static IFQueue Free; // size: 0x8, address: 0xC
static IPRoute RoutingTable[ROUTE_TABLE_SIZE]; // size: 0x500, address: 0x0

// Range: 0x193C -> 0x1958
s32 IPGetConfigError(IPInterface* interface /* r3 */) {
    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    return interface->err;
}

// Range: 0x1958 -> 0x19C0
s32 IPSetConfigError(IPInterface* interface /* r31 */, s32 err /* r1+0xC */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    if (interface->err == 0) {
        interface->err = err;
    }
    OSRestoreInterrupts(enabled);
    return interface->err;
}

// Range: 0x19C0 -> 0x1A1C
s32 IPClearConfigError(IPInterface* interface /* r31 */) {
    // Local variables
    BOOL enabled; // r30
    s32 prev; // r29

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    prev = interface->err;
    interface->err = 0;
    OSRestoreInterrupts(enabled);
    return prev;
}
