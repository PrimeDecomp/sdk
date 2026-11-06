#include <dolphin/private/ip.h>

static u16 Id = 1;
const u8 IPAddrAny[4] = { 0, 0, 0, 0 }; // 0.0.0.0
const u8 IPLoopbackAddr[4] = { 127, 0, 0, 1 }; // 127.0.0.1
const u8 IPLimited[4] = { 255, 255, 255, 255 }; // 255.255.255.255

u16 IPCheckSum(IPHeader* ip) {
    int len;
    u32 sum;
    u16* p;

    sum = 0;
    len = IP_HLEN(ip);
    p = (u16*)ip;
    
    for (; len > 0; len -= sizeof(u16)) {
        sum += *p++;
    }

    /* Add the 16-bit carry values twice and return the inverse */
    sum = (sum & 0xFFFF) + ((sum >> 16) & 0xFFFF);
    sum = (sum & 0xFFFF) + ((sum >> 16) & 0xFFFF);
    return sum ^ 0xFFFF;
}

void IPIn(IPInterface* interface, IPHeader* ip, s32 len, u32 flag) {
    BOOL bcast;

    bcast = IPIsBroadcastAddr(interface, ip->dst);
    if (len < 20 || len < ip->len) {
        return;
    }

    if ((flag & 1) != 0 && !bcast) {
        return;
    }

    if ((flag & 2) != 0 && !IP_CLASSD(ip->dst)) {
        return;
    }

    if ((ip->verlen >> 4) != 4 || ip->len < IP_HLEN(ip)) {
        return;
    }

    if (IPCheckSum(ip) != 0) {
        return;
    }

    if (IP_CLASSD(ip->src) || (IPNEQ(ip->src, IPAddrAny) && IPIsBroadcastAddr(interface, ip->src))) {
        return;
    }

    if (IP_CLASSE(ip->dst) || IP_CLASSE(ip->src)) {
        return;
    }

    if (IPProcessSourceRoute(ip) < 0) {
        return;
    }

    if (IPNEQ(interface->addr, IPAddrAny) && !IP_CLASSD(ip->dst) && !bcast && ip->dst[0] != 127 &&
        IPNEQ(ip->dst, interface->addr) && IPNEQ(ip->dst, interface->alias)) {
        return;
    }

    if ((ip->frag & IP_HAS_FRAG) != 0 || IP_FRAG(ip) != 0) {
        ip = IPReassemble(interface, ip, flag);
        if (ip == NULL) {
            return;
        }
    }
    
    switch (ip->proto) {
        case IP_PROTO_ICMP:
            ICMPIn(interface, ip, flag);
            break;
        case IP_PROTO_IGMP:
            IGMPIn(interface, ip, flag);
            break;
        case IP_PROTO_UDP:
            UDPIn(interface, ip, flag);
            break;
        case IP_PROTO_TCP:
            if ((flag & 0x3) == 0) {
                TCPIn(interface, ip, flag);
            }
            break;
    }

    ip->verlen = 0;
}

s32 IPOut(IFDatagram* datagram) {
    IPHeader* ip;
    IPInterface* interface;
    TCPHeader* tcp;
    UDPHeader* udp;
    IGMP* igmp;

    ASSERTLINE(1035, 0 < datagram->nVec && datagram->nVec <= IF_MAX_VEC);
    ip = (IPHeader*)datagram->vec[0].data;
    ASSERTLINE(1037, IP_HLEN(ip) <= datagram->vec[0].len);

    ip->id = Id++;
    if (IP_CLASSD(ip->dst)) {
        interface = &__IFDefault;
        memmove(datagram->dst, ip->dst, sizeof(datagram->dst));
    } else {
        interface = (IPInterface*)IPGetRoute(ip->dst, datagram->dst);
        if (interface == NULL) {
            return -2;
        }
    }

    if (interface->mtu < ip->len && (ip->frag & IP_DONT_FRAG) != 0) {
        return -17;
    }

    if (ip->src[0] != 127 && IPNEQ(interface->addr, ip->src) && IPNEQ(interface->alias, ip->src)) {
        return -19;
    }


    ip->sum = 0;
    ip->sum = IPCheckSum(ip);

    switch (ip->proto) {
        case IP_PROTO_IGMP:
            igmp = (IGMP*)(((u8*)ip) + IP_HLEN(ip));
            igmp->sum = 0;
            igmp->sum = IGMPCheckSum(igmp);
            break;
        case IP_PROTO_UDP:
            udp = (UDPHeader*)(((u8*)ip) + IP_HLEN(ip));
            udp->sum = 0;
            udp->sum = UDPCheckSum(datagram->vec, datagram->nVec);
            if (udp->sum == 0) {
                udp->sum = 0xFFFF;
            }
            break;
        case IP_PROTO_TCP:
            tcp = (TCPHeader*)(((u8*)ip) + IP_HLEN(ip));
            tcp->sum = 0;
            tcp->sum = TCPCheckSum(datagram->vec, datagram->nVec);
            ASSERTLINE(1098, (tcp->flag & (TCP_FLAG_SYN | TCP_FLAG_FIN)) != (TCP_FLAG_SYN | TCP_FLAG_FIN));
            break;
    }

    datagram->type = ETH_IP;
    (*interface->out)(interface, datagram);
    return 0;
}

void IPCancel(IFDatagram* datagram) {
    IPInterface* interface;

    interface = datagram->interface;
    if (interface) {
        (*interface->cancel)(interface, datagram);
        ASSERTLINE(1119, datagram->interface == NULL);
    }
}
