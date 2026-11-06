#include <dolphin/private/ip.h>

static u16 Id = 1;
const u8 IPAddrAny[4] = { 0, 0, 0, 0 }; // 0.0.0.0
const u8 IPLoopbackAddr[4] = { 127, 0, 0, 1 }; // 127.0.0.1
const u8 IPLimited[4] = { 255, 255, 255, 255 }; // 255.255.255.255

IPInfo* IPLookupInfo(IFQueue* queue, u8* srcAddr, u8* dstAddr, u16 src, u16 dst, u32 flag) {
    IPInfo* info;
    IPInfo* next;
    int wildcard;
    int minimum;
    IPInfo* match;
    s32 mcast;

    minimum = 3;
    match = NULL;

    if (IP_CLASSD(dstAddr)) {
        mcast = IPMulticastLookup(dstAddr, IPAddrAny);

        if (mcast < 0) {
            return NULL;
        }
    }

    IFQueueIterator(IPInfo*, queue, info, next) {
        if (
            (info->local.port != 0 && info->local.port == dst) && (!IP_CLASSD(dstAddr) ||
            ((info->flag & (1 << mcast)) != 0 && (((flag & 0x4) == 0) || ((info->flag & 0x8000) != 0))))
        ) {
            wildcard = 0;
            if (IPNEQ(dstAddr, IPAddrAny)) {
                if (IPEQ(info->local.addr, IPAddrAny)) {
                    wildcard++;
                } else if (IPNEQ(info->local.addr, dstAddr)) {
                    continue;
                }
            } else if (IPNEQ(info->local.addr, IPAddrAny)) {
                wildcard++;
            }
            
            if (IPNEQ(srcAddr, IPAddrAny)) {
                if (IPEQ(info->remote.addr, IPAddrAny)) {
                    wildcard++;
                } else if (info->remote.port != src || IPNEQ(info->remote.addr, srcAddr)) {
                    continue;
                }
            } else if (IPNEQ(info->remote.addr, IPAddrAny)) {
                wildcard++;
            }
            
            if (wildcard < minimum) {
                match = info;
                minimum = wildcard;

                if (minimum == 0) {
                    break;
                }
            }
        }
    }

    return match;
}

BOOL __IPIsMember(IFQueue* queue, IPInfo* info) {
    IPInfo* iter;
    IPInfo* next;

    IFQueueIterator(IPInfo*, queue, iter, next) {
        if (iter == info) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IPBind(IFQueue* queue, IPInfo* info, const IPSocket* socket, BOOL reuse) {
    IPInfo* iter;
    IPInfo* next;

    if (socket->len != 8 || socket->family != 2 || socket->port == 0 || IP_CLASSE(socket->addr)) {
        return -12;
    }

    if (socket->port == 0 || (IPNEQ(socket->addr, IPAddrAny) && IPNEQ(socket->addr, IPLoopbackAddr) &&
        IPNEQ(socket->addr, __IFDefault.addr) && IPNEQ(socket->addr, __IFDefault.alias) && !IP_CLASSD(socket->addr))) {
        return -13;
    }

    IFQueueIterator(IPInfo*, queue, iter, next) {
        if (iter != info && iter->local.port == socket->port && IPEQ(iter->local.addr, socket->addr) &&
            (reuse == FALSE || (iter->remote.port == info->remote.port && IPEQ(iter->remote.addr, info->remote.addr)))
        ) {
            return -5;
        }
    }

    memmove(&info->local, socket, sizeof(info->local));
    return 0;
}

u16 IPGetAnonPort(IFQueue* queue, u16* last) {
    u16 port;
    IPInfo* info;
    IPInfo* next;
    int skip;

    skip = 0;
    if (*last < 0xC000) {
        *last = 0xC000;
    }

loop:
    port = *last;
    *last = port + 1;
    if (*last < 0xC000) {
        *last = 0xC000;
    }

    IFQueueIterator(IPInfo*, queue, info, next) {
        if (info->local.port == port) {
            if (++skip <= 0x3FFF) {
                goto loop;
            }

            return 0;
        }
    }

    return port;
}

s32 IPConnect(IFQueue* queue, IPInfo* info, const IPSocket* socket, u16* last) {
    IPInfo* iter;
    IPInfo* next;
    IPInterface* interface;
    const u8* localAddr;

    if (socket == NULL || socket->len != 8 || socket->family != 2 || socket->port == 0 || IP_CLASSE(socket->addr)) {
        return -12;
    }

    if (IPEQ(socket->addr, IPAddrAny)) {
        return -13;
    }

    interface = IPGetRoute(socket->addr, NULL);
    if (interface == NULL) {
        return -2;
    }

    if (info->proto == 6 && (IP_CLASSD(socket->addr) || IPIsBroadcastAddr(interface, socket->addr))) {
        return -13;
    }


    if (IPEQ(info->local.addr, IPAddrAny)) {
        if (socket->addr[0] == 127) {
            localAddr = IPLoopbackAddr;
        } else if (memcmp(socket->addr, interface->alias, 2) == 0 || IPEQ(interface->addr, IPAddrAny)) {
            localAddr = interface->alias;
        } else {
            localAddr = interface->addr;
        }
    } else {
        localAddr = info->local.addr;
    }

    if (info->local.port == 0) {
        do {
            info->local.port = IPGetAnonPort(queue, last);
            if (info->local.port == 0) {
                return -7;
            }
        } while (info->proto == IP_PROTO_TCP && TCPLookupTimeWaitInfo(socket->addr, socket->port, localAddr, info->local.port));
    } else {
        IFQueueIterator(IPInfo*, queue, iter, next) {
            if (iter != info && iter->local.port == info->local.port && iter->remote.port == info->remote.port &&
                IPEQ(iter->local.addr, localAddr) && IPEQ(iter->remote.addr, info->remote.addr)) {
                return -5;
            }
        }

        if (info->proto == IP_PROTO_TCP && TCPLookupTimeWaitInfo(socket->addr, socket->port, localAddr, info->local.port)) {
            return -5;
        }
    }

    memmove(&info->remote, socket, sizeof(info->remote));
    if (IPEQ(info->local.addr, IPAddrAny)) {
        memmove(info->local.addr, localAddr, sizeof(info->local.addr));
    }

    return 0;
}

s32 IPGetRemoteSocket(IPInfo* info, IPSocket* socket) {
    ASSERTLINE(668, info->remote.len == IP_SOCKLEN);
    ASSERTLINE(669, info->remote.family == IP_INET);
    memcpy(socket, &info->remote, sizeof(info->remote));
    return 0;
}

s32 IPGetLocalSocket(IPInfo* info, IPSocket* socket) {
    ASSERTLINE(677, info->local.len == IP_SOCKLEN);
    ASSERTLINE(678, info->local.family == IP_INET);
    memcpy(socket, &info->local, sizeof(info->local));
    return 0;
}

s32 IPGetSockOpt(IPInfo* info, int level, int optname, void* optval, int* optlen) {
    BOOL enabled;
    s32 rc;

    rc = -14;
    enabled = OSDisableInterrupts();
    if (level == 0) {
        switch (optname) {
            case IP_OPT_TOS:
                if (*(u32*)optlen >= sizeof(u32)) {
                    *(u32*)optval = info->tos;
                    *optlen = sizeof(u32);
                    rc = 0;
                } else {
                    rc = -12;
                }
                break;
            case IP_OPT_TTL:
                if (*(u32*)optlen >= sizeof(u32)) {
                    *(u32*)optval = info->ttl;
                    *optlen = sizeof(u32);
                    rc = 0;
                } else {
                    rc = -12;
                }
                break;
            case IP_OPT_9:
                if (*(u32*)optlen >= sizeof(u8)) {
                    *(u8*)optval = (info->flag & 0x8000) ? TRUE : FALSE;
                    *optlen = sizeof(u8);
                    rc = 0;
                } else {
                    rc = -12;
                }
                break;
            case IP_OPT_MTTL:
                if (*(u32*)optlen >= sizeof(info->mttl)) {
                    *(u8*)optval = info->mttl;
                    *optlen = sizeof(info->mttl);
                    rc = 0;
                } else {
                    rc = -12;
                }
                break;
        }
    }

    OSRestoreInterrupts(enabled);
    return rc;
}

s32 IPSetSockOpt(IPInfo* info, int level, int optname, void* optval, int optlen) {
    BOOL enabled;
    s32 rc;

    rc = -14;
    enabled = OSDisableInterrupts();

    if (level == 0) {
        switch (optname) {
            case IP_OPT_TOS:
                if (optlen >= sizeof(u32)) {
                    info->tos = *(s32*)optval < 255 ? *(u32*)optval : 255;
                    rc = 0;
                } else {
                    rc = -12;
                }
                break;
            case IP_OPT_TTL:
                if (optlen >= sizeof(u32)) {
                    info->ttl = *(s32*)optval < 255 ? *(u32*)optval : 255;
                    rc = 0;
                } else {
                    rc = -12;
                }
                break;
            case IP_OPT_9:
                if (optlen >= sizeof(u8)) {
                    if (*(u8*)optval) {
                        info->flag |= 0x8000;
                    } else {
                        info->flag &= ~0x8000;
                    }

                    rc = 0;
                } else {
                    rc = -12;
                }
                break;
            case IP_OPT_MTTL:
                if (optlen >= sizeof(info->mttl)) {
                    info->mttl = *(u8*)optval;
                    rc = 0;
                } else {
                    rc = -12;
                }
                break;
            case IP_OPT_JOIN_MCAST:
                if (optlen >= sizeof(SOIpMreq)) {
                    const SOIpMreq* mreq = (const SOIpMreq*)optval;

                    rc = IPMulticastLookup((u8*)&mreq->multiaddr.addr, (u8*)&mreq->interface.addr);
                    if (0 <= rc) {
                        if ((info->flag & (1 << rc)) == 0) {
                            rc = IPMulticastJoin((u8*)&mreq->multiaddr.addr, (u8*)&mreq->interface.addr);
                            ASSERTLINE(822, 0 <= rc);
                            info->flag |= 1 << rc;
                        }

                        rc = 0;
                    } else {
                        rc = IPMulticastJoin((u8*)&mreq->multiaddr.addr,(u8*)&mreq->interface.addr);
                        if (0 <= rc) {
                            info->flag |= 1 << rc;
                            rc = 0;
                        }
                    }
                } else {
                    rc = -12;
                }
                break;
            case IP_OPT_LEAVE_MCAST:
                if (optlen >= sizeof(SOIpMreq)) {
                    const SOIpMreq* mreq = (const SOIpMreq*)optval;

                    rc = IPMulticastLookup((u8*)&mreq->multiaddr.addr, (u8*)&mreq->interface.addr);
                    if (0 <= rc) {
                        if ((info->flag & (1 << rc)) != 0) {
                            rc = IPMulticastLeave((u8*)&mreq->multiaddr.addr, (u8*)&mreq->interface.addr);
                            ASSERTLINE(855, 0 <= rc);
                            info->flag &= ~(1 << rc);
                            rc = 0;
                        } else {
                            rc = -4;
                        }
                    }
                } else {
                    rc = -12;
                }
                break;
        }
    }

    OSRestoreInterrupts(enabled);
    return rc;
}

BOOL IPSetOption(IPInfo* info, u8 ttl, u8 tos) {
    info->ttl = ttl;
    info->tos = tos;
    return TRUE;
}
