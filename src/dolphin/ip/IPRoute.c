#include <dolphin/ip.h>
#include <dolphin/ip/IPArp.h>
#include <dolphin/os/OSPriv.h>
#include <dolphin/private/ip.h>

#define ROUTE_TABLE_SIZE 32

#define ROUTE_GATEWAY  0x1
#define ROUTE_DYNAMIC  0x2
#define ROUTE_MODIFIED 0x4

static BOOL Initialized;
static IFQueue Up;
static IFQueue Free;
static IPRoute RoutingTable[ROUTE_TABLE_SIZE];

static int CountBits(const u8* addr) {
    int count;
    int i;
    u8 mask;

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

void IPGetMacAddr(IPInterface* interface, u8* macAddr) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(macAddr, interface->mac, 6);
    OSRestoreInterrupts(enabled);
}

void IPGetBroadcastAddr(IPInterface* interface, u8* broadcastAddr) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(broadcastAddr, interface->broadcast, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPGetNetmask(IPInterface* interface, u8* netmask) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(netmask, interface->netmask, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPGetAddr(IPInterface* interface, u8* addr) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(addr, interface->addr, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPGetGateway(IPInterface* interface, u8* gateway) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(gateway, interface->gateway, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPGetAlias(IPInterface* interface, u8* alias) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(alias, interface->alias, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPGetMtu(IPInterface* interface, s32* mtu) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    *mtu = interface->mtu;
    OSRestoreInterrupts(enabled);
}

void IPGetLinkState(IPInterface* interface, BOOL* up) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    *up = interface->up;
    OSRestoreInterrupts(enabled);
}

void IPGetInterfaceStat(IPInterface* interface, IPInterfaceStat* stat) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    if (stat) {
        enabled = OSDisableInterrupts();
        memmove(stat, &interface->stat, sizeof(IPInterfaceStat));
        OSRestoreInterrupts(enabled);
    }
}

void IPClearInterfaceStat(IPInterface* interface) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memset(&interface->stat, 0, sizeof(IPInterfaceStat));
    OSRestoreInterrupts(enabled);
}

void IPSetBroadcastAddr(IPInterface* interface, const u8* broadcastAddr) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->broadcast, broadcastAddr ? broadcastAddr : IPLimited, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPSetNetmask(IPInterface* interface, const u8* netmask) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->netmask, netmask ? netmask : IPAddrAny, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPSetAddr(IPInterface* interface, const u8* addr) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->addr, addr ? addr : IPAddrAny, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPSetGateway(IPInterface* interface, const u8* gateway) {
    BOOL enabled;

    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->gateway, gateway ? gateway : IPAddrAny, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPSetAlias(IPInterface* interface, const u8* alias) {
    BOOL enabled;

    ASSERTLINE(273, alias == NULL || alias[0] == 169 && alias[1] == 254 || IP_ADDR_EQ(alias, IPAddrAny));
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    memcpy(interface->alias, alias ? alias : IPAddrAny, IP_ALEN);
    OSRestoreInterrupts(enabled);
}

void IPSetMtu(IPInterface* interface, s32 mtu) {
    BOOL enabled;

    ASSERTLINE(284, 68 <= mtu);
    interface = interface ? interface : &__IFDefault;
    enabled = OSDisableInterrupts();
    interface->mtu = (mtu < 68) ? 68 : mtu;
    OSRestoreInterrupts(enabled);
}

void IPPrintRoutingTable(void) {
    IPRoute* route;
    IPRoute* next;

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

static BOOL Hit(const u8* dst, const u8* addr, const u8* netmask) {
    int i;

    for (i = 0; i < IP_ALEN; i++) {
        if (dst[i] != (addr[i] & netmask[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL IPIsBroadcastAddr(IPInterface* interface, const u8* addr) {
    u32 netmask;
    u32 netid;
    u32 ipaddr;

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
    if (netid != 0) {
        if (netid == (ipaddr & netmask) && ~netmask == (ipaddr & ~netmask)) {
            return TRUE;
        }
        if (IP_CLASSA(interface->addr)) {
            netmask = 0xFF000000;
        } else if (IP_CLASSB(interface->addr)) {
            netmask = 0xFFFF0000;
        } else if (IP_CLASSC(interface->addr)) {
            netmask = 0xFFFFFF00;
        }
        if ((IPU32(interface->addr) & netmask) == (ipaddr & netmask) && ~netmask == (ipaddr & ~netmask)) {
            return TRUE;
        }
    }

    netmask = 0xFFFF0000;
    netid = IPU32(interface->alias) & netmask;
    if (netid != 0 && netid == (ipaddr & netmask) && (ipaddr & ~netmask) == ~netmask) {
        return TRUE;
    }
    return FALSE;
}

BOOL IPIsLoopbackAddr(IPInterface*, const u8* addr) {
    return (addr[0] == 127) ? TRUE : FALSE;
}

BOOL IPIsLocalAddr(IPInterface* interface, const u8* addr) {
    u8 classB[4] = { 255, 255, 0, 0 };
    BOOL enabled;
    BOOL rc;

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

IPInterface* IPGetRoute(const u8* addr, u8* dst) {
    IPRoute* route;
    IPRoute* next;

    IFQueueIterator(IPRoute*, &Up, route, next) {
        if (Hit(route->dst, addr, route->netmask)) {
            if (dst) {
                if (route->flag & ROUTE_GATEWAY) {
                    memmove(dst, route->gateway, IP_ALEN);
                } else {
                    memmove(dst, addr, IP_ALEN);
                }
            }
            route->time = __OSGetSystemTime();
            return route->interface;
        }
    }
    return NULL;
}

static BOOL AddRoute(const u8* dst, const u8* netmask, const u8* gateway, u32 flag) {
    IPRoute* route;
    IPRoute* next;
    IPRoute* ent;
    int i;
    int bits;

    if (!Hit(dst, dst, netmask)) {
        return FALSE;
    }

    if (IPEQ(gateway, IPAddrAny)) {
        flag &= ~ROUTE_GATEWAY;
    } else {
        flag |= ROUTE_GATEWAY;
    }

    IFQueueIterator(IPRoute*, &Up, route, next) {
        for (i = 0; i < IP_ALEN; i++) {
            if ((route->dst[i] & route->netmask[i]) != (dst[i] & netmask[i])) {
                break;
            }
        }
        if (i < IP_ALEN) {
            continue;
        }

        if ((flag & ROUTE_GATEWAY) && !(route->flag & (ROUTE_DYNAMIC | ROUTE_MODIFIED))) {
            if (IPEQ(gateway, route->gateway)) {
                return TRUE;
            }
        } else {
            memmove(route->gateway, gateway, IP_ALEN);
            memmove(route->netmask, netmask, IP_ALEN);
            if (!(route->flag & ROUTE_DYNAMIC)) {
                flag &= ~ROUTE_DYNAMIC;
            }
            route->flag = flag;
            route->time = __OSGetSystemTime();
            route->interface = &__IFDefault;
            return TRUE;
        }
    }

    if (Free.next == NULL) {
        ent = NULL;
        IFQueueIterator(IPRoute*, &Up, route, next) {
            if (route->flag & ROUTE_DYNAMIC) {
                if (ent == NULL) {
                    ent = route;
                } else if (route->time < ent->time) {
                    ent = route;
                }
            }
        }
        if (ent == NULL) {
            return FALSE;
        }

        IFQueueDequeueEntry(IPRoute*, &Up, ent);
    } else {
        IFQueueDequeueHead(IPRoute*, &Free, ent);
    }

    memmove(ent->dst, dst, IP_ALEN);
    memmove(ent->netmask, netmask, IP_ALEN);
    memmove(ent->gateway, gateway, IP_ALEN);
    flag &= ~ROUTE_MODIFIED;
    ent->flag = flag;
    ent->time = __OSGetSystemTime();
    ent->interface = &__IFDefault;

    bits = CountBits(netmask);
    IFQueueIterator(IPRoute*, &Up, route, next) {
        if (CountBits(route->netmask) < bits) {
            ent->link.prev = route->link.prev;
            ent->link.next = (IFQueue*)route;
            route->link.prev = (IFQueue*)ent;
            if (ent->link.prev == NULL) {
                Up.next = (IFQueue*)ent;
            } else {
                ent->link.prev->next = (IFQueue*)ent;
            }
            return TRUE;
        }
    }

    IFQueueEnqueueTail(IPRoute*, &Up, ent);
    return TRUE;
}

BOOL IPRedirect(const u8* dst, const u8* gateway, const u8* router) {
    IPInterface* interface;
    u8 addr[4];

    if (IPEQ(gateway, IPAddrAny)) {
        return FALSE;
    }

    interface = IPGetRoute(dst, addr);
    if (interface == NULL) {
        return FALSE;
    }

    if ((IPU32(gateway) & IPU32(interface->netmask)) != (IPU32(interface->addr) & IPU32(interface->netmask)) ||
        IPIsBroadcastAddr(interface, gateway)) {
        return FALSE;
    }

    if (IPNEQ(addr, router)) {
        return FALSE;
    }

    if (IPGetRoute(gateway, NULL) != IPGetRoute(router, NULL)) {
        return FALSE;
    }

    if (IPEQ(gateway, __IFDefault.addr) || IPEQ(gateway, __IFDefault.alias)) {
        return FALSE;
    }

    return AddRoute(dst, IPLimited, gateway, ROUTE_GATEWAY | ROUTE_DYNAMIC | ROUTE_MODIFIED);
}

BOOL IPAddRoute(const u8* dst, const u8* netmask, const u8* gateway) {
    BOOL enabled;
    BOOL rc;

    enabled = OSDisableInterrupts();
    rc = AddRoute(dst, netmask, gateway, 0);
    OSRestoreInterrupts(enabled);
    return rc;
}

BOOL IPRemoveRoute(const u8* dst, const u8* netmask, const u8* gateway) {
    BOOL enabled;
    BOOL rc;
    IPRoute* route;
    IPRoute* next;

    enabled = OSDisableInterrupts();
    rc = FALSE;
    IFQueueReverseIterator(IPRoute*, &Up, route, next) {
        if (IPEQ(dst, route->dst) && (IPEQ(netmask, IPAddrAny) || IPEQ(netmask, route->netmask)) &&
            (IPEQ(gateway, IPAddrAny) || IPEQ(gateway, route->gateway))) {
            IFQueueDequeueEntry(IPRoute*, &Up, route);

            IFQueueEnqueueHead(IPRoute*, &Free, route);

            rc = TRUE;
            break;
        }
    }
    OSRestoreInterrupts(enabled);
    return rc;
}

BOOL IPInitRoute(const u8* addr, const u8* netmask, const u8* gateway) {
    BOOL enabled;
    IPRoute* route;
    IPRoute* end;
    u8 broadcast[IP_ALEN];
    int i;

    enabled = OSDisableInterrupts();
    if (!Initialized) {
        Initialized = TRUE;
        memset(RoutingTable, 0, sizeof(RoutingTable));
        IFQueueInit(&Up);
        IFQueueInit(&Free);
        end = &RoutingTable[ROUTE_TABLE_SIZE];
        for (route = RoutingTable; route < end; ++route) {
            IFQueueEnqueueTail(IPRoute*, &Free, route);
        }
    }
    OSRestoreInterrupts(enabled);

    IPSetAddr(&__IFDefault, addr);
    IPSetNetmask(&__IFDefault, netmask);
    IPSetGateway(&__IFDefault, gateway);

    if (addr && netmask) {
        for (i = 0; i < IP_ALEN; i++) {
            broadcast[i] = addr[i] & netmask[i];
            broadcast[i] |= ~netmask[i];
        }
        IPSetBroadcastAddr(&__IFDefault, broadcast);
    }

    return IPRefreshRoute();
}

BOOL IPRefreshRoute(void) {
    BOOL enabled;
    IPRoute* route;
    IPRoute* next;
    u8* addr;
    u8* netmask;
    u8* gateway;
    u8* alias;
    u8 loopback[4] = { 127, 0, 0, 0 };
    u8 classA[4] = { 255, 0, 0, 0 };
    u8 classB[4] = { 255, 255, 0, 0 };
    u8 netid[4];
    int i;

    enabled = OSDisableInterrupts();
    addr = __IFDefault.addr;
    netmask = __IFDefault.netmask;
    gateway = __IFDefault.gateway;
    alias = __IFDefault.alias;

    IFQueueIterator(IPRoute*, &Up, route, next) {
        IFQueueDequeueEntry(IPRoute*, &Up, route);

        IFQueueEnqueueHead(IPRoute*, &Free, route);
    }

    IPAddRoute(IPLimited, IPLimited, IPAddrAny);
    if (IPNEQ(addr, IPAddrAny)) {
        IPAddRoute(addr, IPLimited, IPLoopbackAddr);
        for (i = 0; i < IP_ALEN; i++) {
            netid[i] = addr[i] & netmask[i];
        }
        IPAddRoute(netid, netmask, IPAddrAny);
    }
    if (IPNEQ(alias, IPAddrAny)) {
        IPAddRoute(alias, IPLimited, IPLoopbackAddr);
        for (i = 0; i < IP_ALEN; i++) {
            netid[i] = classB[i] & alias[i];
        }
        IPAddRoute(netid, classB, IPAddrAny);
    }
    IPAddRoute(loopback, classA, IPLoopbackAddr);
    if (IPNEQ(gateway, IPAddrAny)) {
        IPAddRoute(IPAddrAny, IPAddrAny, gateway);
    }
    ARPGratuitous(&__IFDefault);
    OSRestoreInterrupts(enabled);
    return TRUE;
}

static void DecayRoute(IPRoute* dead) {
    int bits;
    IPRoute* route;
    IPRoute* next;

    bits = CountBits(dead->netmask);
    IFQueueDequeueEntry(IPRoute*, &Up, dead);

    IFQueueReverseIterator(IPRoute*, &Up, route, next) {
        if (bits <= CountBits(route->netmask)) {
            dead->link.prev = (IFQueue*)route;
            dead->link.next = route->link.next;
            route->link.next = (IFQueue*)dead;
            if (dead->link.next == NULL) {
                Up.prev = (IFQueue*)dead;
            } else {
                dead->link.next->prev = (IFQueue*)dead;
            }
            return;
        }
    }

    IFQueueEnqueueHead(IPRoute*, &Up, dead);
}

BOOL IPRecoverGateway(const u8* dst) {
    IPRoute* route;
    IPRoute* next;

    if (IPEQ(dst, IPAddrAny) || dst[0] == 127) {
        return FALSE;
    }

    IFQueueReverseIterator(IPRoute*, &Up, route, next) {
        if ((route->flag & ROUTE_GATEWAY) && route->gateway[0] != 127 && IPEQ(route->gateway, dst)) {
            DecayRoute(route);
            return TRUE;
        }
    }

    IFQueueIterator(IPRoute*, &Up, route, next) {
        if (Hit(route->dst, dst, route->netmask)) {
            if ((route->flag & ROUTE_GATEWAY) && route->gateway[0] != 127) {
                DecayRoute(route);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}
