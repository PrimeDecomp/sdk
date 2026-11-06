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

// Range: 0x0 -> 0x68
static int CountBits(const u8* addr /* r3 */) {
    // Local variables
    int count; // r29
    int i; // r31
    u8 mask; // r30

    count = 0;
    for (i = 0; i < IP_ALEN; i++) {
        for (mask = 0x80; mask; mask >>= 1) {
            if (addr[i] & mask) {
                count++;
            }
        }
    }
    return count;
}

// Range: 0x68 -> 0xC8
void IPGetMacAddr(IPInterface* interface /* r31 */, u8* macAddr /* r1+0xC */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(macAddr, interface->mac, 6);
    OSRestoreInterrupts(enabled);
}

// Range: 0xC8 -> 0x128
void IPGetBroadcastAddr(IPInterface* interface /* r31 */, u8* broadcastAddr /* r1+0xC */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(broadcastAddr, interface->broadcast, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x128 -> 0x188
void IPGetNetmask(IPInterface* interface /* r31 */, u8* netmask /* r1+0xC */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(netmask, interface->netmask, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x188 -> 0x1E8
void IPGetAddr(IPInterface* interface /* r31 */, u8* addr /* r1+0xC */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(addr, interface->addr, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x1E8 -> 0x248
void IPGetGateway(IPInterface* interface /* r31 */, u8* gateway /* r1+0xC */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(gateway, interface->gateway, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x248 -> 0x2A8
void IPGetAlias(IPInterface* interface /* r31 */, u8* alias /* r1+0xC */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(alias, interface->alias, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x2A8 -> 0x304
void IPGetMtu(IPInterface* interface /* r31 */, s32* mtu /* r1+0xC */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    *mtu = interface->mtu;
    OSRestoreInterrupts(enabled);
}

// Range: 0x304 -> 0x360
void IPGetLinkState(IPInterface* interface /* r31 */, BOOL* up /* r1+0xC */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    *up = interface->up;
    OSRestoreInterrupts(enabled);
}

// Range: 0x360 -> 0x3C8
void IPGetInterfaceStat(IPInterface* interface /* r31 */, IPInterfaceStat* stat /* r29 */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    if (stat) {
        enabled = OSDisableInterrupts();
        memmove(stat, &interface->stat, sizeof(IPInterfaceStat));
        OSRestoreInterrupts(enabled);
    }
}

// Range: 0x3C8 -> 0x424
void IPClearInterfaceStat(IPInterface* interface /* r31 */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memset(&interface->stat, 0, sizeof(IPInterfaceStat));
    OSRestoreInterrupts(enabled);
}

// Range: 0x424 -> 0x494
void IPSetBroadcastAddr(IPInterface* interface /* r31 */, const u8* broadcastAddr /* r29 */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> unsigned char IPLimited[4];
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->broadcast, broadcastAddr ? broadcastAddr : IPLimited, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x494 -> 0x504
void IPSetNetmask(IPInterface* interface /* r31 */, const u8* netmask /* r29 */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> unsigned char IPAddrAny[4];
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->netmask, netmask ? netmask : IPAddrAny, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x504 -> 0x574
void IPSetAddr(IPInterface* interface /* r31 */, const u8* addr /* r29 */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> unsigned char IPAddrAny[4];
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->addr, addr ? addr : IPAddrAny, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x574 -> 0x5E4
void IPSetGateway(IPInterface* interface /* r31 */, const u8* gateway /* r29 */) {
    // Local variables
    BOOL enabled; // r30

    // References
    // -> unsigned char IPAddrAny[4];
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->gateway, gateway ? gateway : IPAddrAny, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x5E4 -> 0x6A4
void IPSetAlias(IPInterface* interface /* r30 */, const u8* alias /* r31 */) {
    // Local variables
    BOOL enabled; // r29

    // References
    // -> unsigned char IPAddrAny[4];
    // -> struct IPInterface __IFDefault;
    ASSERTLINE(273, alias == NULL || alias[0] == 169 && alias[1] == 254 || IP_ADDR_EQ(alias, IPAddrAny));
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->alias, alias ? alias : IPAddrAny, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

// Range: 0x6A4 -> 0x728
void IPSetMtu(IPInterface* interface /* r31 */, s32 mtu /* r30 */) {
    // Local variables
    BOOL enabled; // r29

    // References
    // -> struct IPInterface __IFDefault;
    ASSERTLINE(284, 68 <= mtu);
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    interface->mtu = (mtu < 68) ? 68 : mtu;
    OSRestoreInterrupts(enabled);
}

// Range: 0x728 -> 0x858
void IPPrintRoutingTable() {
    // Local variables
    IPRoute* route; // r31
    IPRoute* next; // r29

    // References
    // -> static struct IFQueue Up;
    OSReport("IP Routing table\n");
    OSReport("Destination        Netmask            Gateway            Flags\n");
    IFQueueIterator(IPRoute*, &Up, route, next) {
        OSReport("%3d.%3d.%3d.%3d    %3d.%3d.%3d.%3d    %3d.%3d.%3d.%3d    ",
                 route->dst[0], route->dst[1], route->dst[2], route->dst[3],
                 route->netmask[0], route->netmask[1], route->netmask[2], route->netmask[3],
                 route->gateway[0], route->gateway[1], route->gateway[2], route->gateway[3]);
        if (route->flag & ROUTE_GATEWAY) {
            OSReport("G");
        }
        if (route->flag & ROUTE_DYNAMIC) {
            OSReport("D");
        }
        if (route->flag & ROUTE_MODIFIED) {
            OSReport("M");
        }
        OSReport("\n");
    }
}

// Range: 0x858 -> 0x8A4
static BOOL Hit(const u8* dst /* r3 */, const u8* addr /* r4 */, const u8* netmask /* r5 */) {
    // Local variables
    int i; // r31

    for (i = 0; i < IP_ALEN; i++) {
        if (dst[i] != (addr[i] & netmask[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

// Range: 0x8A4 -> 0x9DC
BOOL IPIsBroadcastAddr(IPInterface* interface /* r30 */, const u8* addr /* r27 */) {
    // Local variables
    u32 netmask; // r31
    u32 netid; // r28
    u32 ipaddr; // r29

    // References
    // -> unsigned char IPAddrAny[4];
    // -> unsigned char IPLimited[4];
    // -> struct IPInterface __IFDefault;
    interface = interface ? interface : &__IFDefault;
    if (IP_CLASSD(addr) || IP_CLASSE(addr)) {
        return FALSE;
    }
    if (IPEQ(addr, IPLimited) || IPEQ(addr, IPAddrAny) || IPEQ(addr, interface->broadcast)) {
        return TRUE;
    }

    ipaddr = IPU32(addr);
    netmask = IPU32(interface->netmask);
    netid = IPU32(interface->addr) & netmask;
    if (netid != 0 && netid == (ipaddr & netmask) && (~netmask == (ipaddr & ~netmask) || (ipaddr & ~netmask) == 0) &&
        CountBits(interface->netmask) < 31) {
        return TRUE;
    }

    netmask = 0xFFFF0000;
    netid = IPU32(interface->alias) & netmask;
    if (netid != 0 && netid == (ipaddr & netmask) && (~netmask == (ipaddr & ~netmask) || (ipaddr & ~netmask) == 0)) {
        return TRUE;
    }
    return FALSE;
}

// Range: 0x9DC -> 0x9F8
BOOL IPIsLoopbackAddr(IPInterface*, const u8* addr /* r4 */) {
    return (addr[0] == 127) ? TRUE : FALSE;
}

// Range: 0x9F8 -> 0xAC0
BOOL IPIsLocalAddr(IPInterface* interface /* r31 */, const u8* addr /* r28 */) {
    // Local variables
    u8 classB[4] = { 255, 255, 0, 0 }; // r1+0x10
    BOOL enabled; // r29
    BOOL rc; // r30

    // References
    // -> unsigned char IPAddrAny[4];
    // -> struct IPInterface __IFDefault;
    rc = FALSE;
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    if (IPNEQ(interface->addr, IPAddrAny) &&
        (IPU32(addr) & IPU32(interface->netmask)) == (IPU32(interface->addr) & IPU32(interface->netmask))) {
        rc = TRUE;
    }
    if (IPNEQ(interface->alias, IPAddrAny) && (IPU32(addr) & IPU32(classB)) == (IPU32(interface->alias) & IPU32(classB))) {
        rc = TRUE;
    }
    OSRestoreInterrupts(enabled);
    return rc;
}
