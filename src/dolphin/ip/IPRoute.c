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

// Range: 0xAC0 -> 0xB9C
IPInterface* IPGetRoute(const u8* addr /* r28 */, u8* dst /* r29 */) {
    // Local variables
    IPRoute* route; // r31
    IPRoute* next; // r30

    // References
    // -> static struct IFQueue Up;
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

// Range: 0xB9C -> 0xF6C
static BOOL AddRoute(const u8* dst /* r20 */, const u8* netmask /* r22 */, const u8* gateway /* r21 */, u32 flag /* r23 */) {
    // Local variables
    IPRoute* route; // r31
    IPRoute* next; // r27
    IPRoute* ent; // r30
    int i; // r24
    int bits; // r19
    // IFQueue* ___next; // r29 (IFQueueDequeueEntry)
    // IFQueue* ___prev; // r28 (IFQueueDequeueEntry)
    // IFQueue* ___next; // r26 (IFQueueDequeueHead)
    // IFQueue* ___prev; // r25 (IFQueueEnqueueTail)

    // References
    // -> static struct IFQueue Up;
    // -> struct IPInterface __IFDefault;
    // -> static struct IFQueue Free;
    // -> unsigned char IPAddrAny[4];
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

        do {
            register IFQueue* ___next;
            register IFQueue* ___prev;

            ___next = ent->link.next;
            ___prev = ent->link.prev;
            if (___next == NULL) {
                Up.prev = ___prev;
            } else {
                ((IPRoute*)___next)->link.prev = ___prev;
            }
            if (___prev == NULL) {
                Up.next = ___next;
            } else {
                ((IPRoute*)___prev)->link.next = ___next;
            }
        } while (0);
    } else {
        do {
            register IFQueue* ___next;

            ent = (IPRoute*)Free.next;
            ___next = ((IPRoute*)ent)->link.next;
            if (___next == NULL) {
                Free.prev = NULL;
            } else {
                ((IPRoute*)___next)->link.prev = NULL;
            }
            Free.next = ___next;
        } while (0);
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

    do {
        register IFQueue* ___prev;

        ___prev = Up.prev;
        if (___prev == NULL) {
            Up.next = (IFQueue*)ent;
        } else {
            ((IPRoute*)___prev)->link.next = (IFQueue*)ent;
        }
        ent->link.prev = ___prev;
        ent->link.next = NULL;
        Up.prev = (IFQueue*)ent;
    } while (0);
    return TRUE;
}

// Range: 0xF6C -> 0x10A4
BOOL IPRedirect(const u8* dst /* r28 */, const u8* gateway /* r31 */, const u8* router /* r29 */) {
    // Local variables
    IPInterface* interface; // r30
    u8 addr[4]; // r1+0x14

    // References
    // -> unsigned char IPLimited[4];
    // -> struct IPInterface __IFDefault;
    // -> unsigned char IPAddrAny[4];
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

// Range: 0x10A4 -> 0x1100
BOOL IPAddRoute(const u8* dst /* r1+0x8 */, const u8* netmask /* r1+0xC */, const u8* gateway /* r1+0x10 */) {
    // Local variables
    BOOL enabled; // r31
    BOOL rc; // r30

    enabled = OSDisableInterrupts();
    rc = AddRoute(dst, netmask, gateway, 0);
    OSRestoreInterrupts(enabled);
    return rc;
}

// Range: 0x1100 -> 0x125C
BOOL IPRemoveRoute(const u8* dst /* r1+0x8 */, const u8* netmask /* r23 */, const u8* gateway /* r24 */) {
    // Local variables
    BOOL enabled; // r25
    BOOL rc; // r27
    IPRoute* route; // r31
    IPRoute* next; // r26
    // IFQueue* ___next; // r30 (IFQueueDequeueEntry)
    // IFQueue* ___prev; // r29 (IFQueueDequeueEntry)
    // IFQueue* ___next; // r28 (IFQueueEnqueueHead)

    // References
    // -> static struct IFQueue Free;
    // -> static struct IFQueue Up;
    // -> unsigned char IPAddrAny[4];
    enabled = OSDisableInterrupts();
    rc = FALSE;
    IFQueueReverseIterator(IPRoute*, &Up, route, next) {
        if (IPEQ(dst, route->dst) && (IPEQ(netmask, IPAddrAny) || IPEQ(netmask, route->netmask)) &&
            (IPEQ(gateway, IPAddrAny) || IPEQ(gateway, route->gateway))) {
            do {
                register IFQueue* ___next;
                register IFQueue* ___prev;

                ___next = route->link.next;
                ___prev = route->link.prev;
                if (___next == NULL) {
                    Up.prev = ___prev;
                } else {
                    ((IPRoute*)___next)->link.prev = ___prev;
                }
                if (___prev == NULL) {
                    Up.next = ___next;
                } else {
                    ((IPRoute*)___prev)->link.next = ___next;
                }
            } while (0);

            do {
                register IFQueue* ___next;

                ___next = Free.next;
                if (___next == NULL) {
                    Free.prev = (IFQueue*)route;
                } else {
                    ((IPRoute*)___next)->link.prev = (IFQueue*)route;
                }
                route->link.next = ___next;
                route->link.prev = NULL;
                Free.next = (IFQueue*)route;
            } while (0);

            rc = TRUE;
            break;
        }
    }
    OSRestoreInterrupts(enabled);
    return rc;
}

// Range: 0x125C -> 0x1478
BOOL IPInitRoute(const u8* addr /* r1+0x8 */, const u8* netmask /* r1+0xC */, const u8* gateway /* r1+0x10 */) {
    // Local variables
    BOOL enabled; // r25
    IPRoute* route; // r29
    IPRoute* end; // r24
    IPInterface* interface; // r31
    int prefix; // r26
    // IFQueue* ___prev; // r30 (IFQueueEnqueueTail)
    u8 broadcast[4]; // r1+0x14
    int i; // r28

    // References
    // -> unsigned char IPAddrAny[4];
    // -> static struct IFQueue Free;
    // -> static struct IPRoute RoutingTable[32];
    // -> static struct IFQueue Up;
    // -> static int Initialized;
    // -> struct IPInterface __IFDefault;
    interface = &__IFDefault;
    enabled = OSDisableInterrupts();
    if (!Initialized) {
        Initialized = TRUE;
        memset(RoutingTable, 0, sizeof(RoutingTable));
        IFQueueInit(&Up);
        IFQueueInit(&Free);
        end = &RoutingTable[ROUTE_TABLE_SIZE];
        for (route = RoutingTable; route < end; ++route) {
            do {
                register IFQueue* ___prev;

                ___prev = Free.prev;
                if (___prev == NULL) {
                    Free.next = (IFQueue*)route;
                } else {
                    ((IPRoute*)___prev)->link.next = (IFQueue*)route;
                }
                route->link.prev = ___prev;
                route->link.next = NULL;
                Free.prev = (IFQueue*)route;
            } while (0);
        }
    }
    OSRestoreInterrupts(enabled);

    IPSetAddr(interface, addr);
    IPSetNetmask(interface, netmask);
    IPSetGateway(interface, gateway);
    IPSetBroadcastAddr(interface, NULL);

    prefix = CountBits(interface->netmask);
    if (prefix < 31) {
        for (i = 0; i < IP_ALEN; i++) {
            broadcast[i] = interface->addr[i] & interface->netmask[i];
            broadcast[i] |= ~interface->netmask[i];
        }
        IPSetBroadcastAddr(interface, broadcast);
    }

    if (IPIsBroadcastAddr(interface, interface->addr) || IPEQ(interface->gateway, interface->addr) ||
        (IPNEQ(interface->gateway, IPAddrAny) &&
         (IPIsBroadcastAddr(interface, interface->gateway) ||
          (prefix != 32 && (IPU32(interface->gateway) & IPU32(interface->netmask)) != (IPU32(interface->addr) & IPU32(interface->netmask)))))) {
        IPSetAddr(interface, NULL);
        IPSetNetmask(interface, NULL);
        IPSetGateway(interface, NULL);
        IPSetBroadcastAddr(interface, NULL);
    }

    return IPRefreshRoute();
}

// Range: 0x1478 -> 0x16AC
BOOL IPRefreshRoute() {
    // Local variables
    BOOL enabled; // r21
    IPRoute* route; // r31
    IPRoute* next; // r24
    u8* addr; // r26
    u8* netmask; // r23
    u8* gateway; // r22
    u8* alias; // r25
    u8 loopback[4] = { 127, 0, 0, 0 }; // r1+0x14
    u8 classA[4] = { 255, 0, 0, 0 }; // r1+0x10
    u8 classB[4] = { 255, 255, 0, 0 }; // r1+0xC
    u8 netid[4]; // r1+0x8
    int i; // r30
    // IFQueue* ___next; // r29 (IFQueueDequeueEntry)
    // IFQueue* ___prev; // r28 (IFQueueDequeueEntry)
    // IFQueue* ___next; // r27 (IFQueueEnqueueHead)

    // References
    // -> struct IPInterface __IFDefault;
    // -> unsigned char IPAddrAny[4];
    // -> unsigned char IPLoopbackAddr[4];
    // -> unsigned char IPLimited[4];
    // -> static struct IFQueue Free;
    // -> static struct IFQueue Up;
    enabled = OSDisableInterrupts();
    addr = __IFDefault.addr;
    netmask = __IFDefault.netmask;
    gateway = __IFDefault.gateway;
    alias = __IFDefault.alias;

    IFQueueIterator(IPRoute*, &Up, route, next) {
        do {
            register IFQueue* ___next;
            register IFQueue* ___prev;

            ___next = route->link.next;
            ___prev = route->link.prev;
            if (___next == NULL) {
                Up.prev = ___prev;
            } else {
                ((IPRoute*)___next)->link.prev = ___prev;
            }
            if (___prev == NULL) {
                Up.next = ___next;
            } else {
                ((IPRoute*)___prev)->link.next = ___next;
            }
        } while (0);

        do {
            register IFQueue* ___next;

            ___next = Free.next;
            if (___next == NULL) {
                Free.prev = (IFQueue*)route;
            } else {
                ((IPRoute*)___next)->link.prev = (IFQueue*)route;
            }
            route->link.next = ___next;
            route->link.prev = NULL;
            Free.next = (IFQueue*)route;
        } while (0);
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

// Range: 0x16AC -> 0x17E0
static void DecayRoute(IPRoute* dead /* r31 */) {
    // Local variables
    int bits; // r25
    IPRoute* route; // r30
    IPRoute* next; // r26
    // IFQueue* ___next; // r29 (IFQueueDequeueEntry)
    // IFQueue* ___prev; // r28 (IFQueueDequeueEntry)
    // IFQueue* ___next; // r27 (IFQueueEnqueueHead)

    // References
    // -> static struct IFQueue Up;
    bits = CountBits(dead->netmask);
    do {
        register IFQueue* ___next;
        register IFQueue* ___prev;

        ___next = dead->link.next;
        ___prev = dead->link.prev;
        if (___next == NULL) {
            Up.prev = ___prev;
        } else {
            ((IPRoute*)___next)->link.prev = ___prev;
        }
        if (___prev == NULL) {
            Up.next = ___next;
        } else {
            ((IPRoute*)___prev)->link.next = ___next;
        }
    } while (0);

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

    do {
        register IFQueue* ___next;

        ___next = Up.next;
        if (___next == NULL) {
            Up.prev = (IFQueue*)dead;
        } else {
            ((IPRoute*)___next)->link.prev = (IFQueue*)dead;
        }
        dead->link.next = ___next;
        dead->link.prev = NULL;
        Up.next = (IFQueue*)dead;
    } while (0);
}

// Range: 0x17E0 -> 0x193C
BOOL IPRecoverGateway(const u8* dst /* r29 */) {
    // Local variables
    IPRoute* route; // r31
    IPRoute* next; // r30

    // References
    // -> static struct IFQueue Up;
    // -> unsigned char IPAddrAny[4];
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
