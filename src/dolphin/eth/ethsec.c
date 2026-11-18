#include <dolphin/eth.h>
#include <dolphin/md5.h>
#include <dolphin/os.h>
#include <dolphin/os/OSMemory.h>
#include <dolphin/private/ip.h>
#include <string.h>

typedef struct TCPResetInfo {
    u8 addr[6];
    u8 dstAddr[4];
    u8 srcAddr[4];
    u16 dstPort;
    u16 srcPort;
    s32 seq;
    s32 ack;
    u16 flag;
    u16 id;
} TCPResetInfo;

enum { DIGEST_SIZE = 16, DIGEST_TEXT_SIZE = 24 };

static const u8 NintendoAddr[6] = {0x00, 0x09, 0xBF, 0x00, 0x00, 0x00};
static TCPResetInfo Ri;
static u8 SendBuf[sizeof(ETHHeader) + IP_MIN_HLEN + TCP_MIN_HLEN + DIGEST_TEXT_SIZE + 4]
    ATTRIBUTE_ALIGN(32);

static u16 CalcIpCheckSum(IPHeader* ip) {
    int len;
    u32 sum;
    u16* p;

    sum = 0;
    len = IP_HLEN(ip);
    p = (u16*)ip;
    for (; len > 0; len -= sizeof(u16)) {
        sum += *p++;
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum = (sum & 0xFFFF) + (sum >> 16);
    return sum ^ 0xFFFF;
}

static u16 CalcTcpCheckSum(IPHeader* ip, s32 len) {
    s32 hlen;
    u16* p;
    u32 sum = 0;

    ASSERTLINE(212, IP_MIN_HLEN + TCP_MIN_HLEN <= len);
    ASSERTLINE(215, ip->proto == IP_PROTO_TCP);
    hlen = IP_HLEN(ip);
    sum += ((u16*)ip->src)[0];
    sum += ((u16*)ip->src)[1];
    sum += ((u16*)ip->dst)[0];
    sum += ((u16*)ip->dst)[1];
    sum += IP_PROTO_TCP;
    sum += len - hlen;
    p = (u16*)((u8*)ip + hlen);
    for (len -= hlen; len > 1; len -= sizeof(u16)) {
        sum += *p++;
    }
    if (len == 1) {
        sum += (u32)*(u8*)p << 8;
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum = (sum & 0xFFFF) + (sum >> 16);
    return sum ^ 0xFFFF;
}

static inline s32 GetTcpSegLen(IPHeader* ip, TCPHeader* tcp) {
    s32 len = ip->len - IP_HLEN(ip) - TCP_HLEN(tcp);
    if (tcp->flag & TCP_FLAG_SYN) {
        ++len;
    }
    if (tcp->flag & TCP_FLAG_FIN) {
        ++len;
    }
    return len;
}

static inline BOOL VerifyPacket(IPHeader* ip) {
    if ((ip->verlen >> 4) != 4 || ip->len < IP_HLEN(ip)) {
        return TRUE;
    }
    if (CalcIpCheckSum(ip) != 0) {
        return TRUE;
    }
    if (IP_CLASSD(ip->dst) || IP_CLASSD(ip->src) || IP_CLASSE(ip->dst) || IP_CLASSE(ip->src)) {
        return TRUE;
    }
    if (CalcTcpCheckSum(ip, ip->len) != 0) {
        return TRUE;
    }
    return FALSE;
}

static inline void CalcDigest(u8 digest[DIGEST_SIZE], TCPHeader* tcp) {
    MD5Context context;
    s64 secret;

    MD5Init(&context);
    MD5Update(&context, (u8*)&tcp->seq, sizeof(tcp->seq));
    MD5Update(&context, (u8*)&tcp->ack, sizeof(tcp->ack));
    secret = ((s64)(tcp->seq ^ tcp->ack) * 0x5DEECE66DLL + 11) % 0x1000000000000LL;
    MD5Update(&context, (u8*)&secret + 2, sizeof(secret) - 2);
    MD5Final(digest, &context);
}

static inline BOOL CheckConsoleType(void) {
    if (__OSGetDIConfig() == 0xFF) {
        switch (*(vu16*)OSPhysicalToCached(0x30E6)) {
        case 0x8200:
            return TRUE;
        case 0x8001:
            if (OSGetPhysicalMemSize() == 48 * 1024 * 1024) {
                return TRUE;
            }
            return FALSE;
        default:
            return FALSE;
        }
    }
    if ((OSGetConsoleType() & OS_CONSOLE_MASK) == OS_CONSOLE_RETAIL) {
        return FALSE;
    }
    return TRUE;
}

BOOL __ETHFilter(u8* buf, s32 len) {
    ETHHeader* eh = (ETHHeader*)buf;
    IPHeader* ip;
    TCPHeader* tcp;
    TCPResetInfo* ri = &Ri;
    u8 digest1[DIGEST_SIZE];
    u8 digest2[DIGEST_SIZE];

    if (CheckConsoleType()) {
        return TRUE;
    }
    if (eh->type != ETH_IP) {
        return TRUE;
    }
    if (memcmp(eh->src, NintendoAddr, 3) == 0) {
        return TRUE;
    }
    len -= sizeof(ETHHeader);
    ip = (IPHeader*)(eh + 1);
    if (len < IP_MIN_HLEN || len < ip->len) {
        return TRUE;
    }
    if (ip->proto != IP_PROTO_TCP) {
        return TRUE;
    }
    tcp = (TCPHeader*)((u8*)ip + IP_HLEN(ip));
    if (TCP_HLEN(tcp) < TCP_MIN_HLEN) {
        return TRUE;
    }
    if (ip->len < IP_HLEN(ip) + TCP_HLEN(tcp)) {
        return TRUE;
    }

    if ((tcp->flag & (TCP_FLAG_SYN | TCP_FLAG_FIN)) == TCP_FLAG_SYN) {
        switch (tcp->src) {
        case 53:
        case 1900:
            return TRUE;
        }
        switch (tcp->dst) {
        case 1900:
            return TRUE;
        }
        if (!IPIsLocalAddr(NULL, ip->src)) {
            return TRUE;
        }
        if ((tcp->flag & TCP_FLAG_RST) != 0) {
            return TRUE;
        }
        if (VerifyPacket(ip)) {
            return TRUE;
        }
        if (ri->flag == 0) {
            memmove(ri->addr, eh->src, sizeof(ri->addr));
            memmove(ri->dstAddr, ip->dst, sizeof(ri->dstAddr));
            memmove(ri->srcAddr, ip->src, sizeof(ri->srcAddr));
            ri->dstPort = tcp->dst;
            ri->srcPort = tcp->src;
            if ((tcp->flag & TCP_FLAG_ACK) == 0) {
                ri->seq = 0;
                ri->ack = tcp->seq + GetTcpSegLen(ip, tcp);
                ri->flag = TCP_FLAG_ACK | TCP_FLAG_SYN | TCP_FLAG_FIN;
            } else {
                ri->seq = tcp->ack;
                ri->ack = 0;
                ri->flag = TCP_FLAG_SYN | TCP_FLAG_FIN;
            }
            ri->id = 0;
            return TRUE;
        }
        ip->len = 0;
        return FALSE;
    } else if ((tcp->flag & (TCP_FLAG_SYN | TCP_FLAG_FIN)) == (TCP_FLAG_SYN | TCP_FLAG_FIN)) {
        if (TCP_HLEN(tcp) != TCP_MIN_HLEN) {
            return TRUE;
        }
        if (ip->len != IP_MIN_HLEN + TCP_MIN_HLEN + DIGEST_TEXT_SIZE) {
            return TRUE;
        }
        if (!IPIsLocalAddr(NULL, ip->src)) {
            return TRUE;
        }
        if (VerifyPacket(ip)) {
            return TRUE;
        }
        CalcDigest(digest1, tcp);
        __IPDecodeFromBase64((const char*)(tcp + 1), DIGEST_TEXT_SIZE, digest2);
        if (memcmp(digest1, digest2, sizeof(digest1)) != 0) {
            return TRUE;
        }
        ip->len = 0;
        return FALSE;
    }
    return TRUE;
}

BOOL __ETHPostSend(u8 ltps, void (*callback)(u8), void* prev) {
    TCPResetInfo* ri = &Ri;
    IPHeader* ip;
    TCPHeader* tcp;
    ETHHeader* eh;
    u8 digest[DIGEST_SIZE];

    if (ri->flag == 0) {
        ri = NULL;
    } else if (prev != SendBuf) {
        ip = (IPHeader*)((ETHHeader*)prev + 1);
        tcp = (TCPHeader*)(ip + 1);
        if (IPEQ(ip->dst, ri->srcAddr) && IPEQ(ip->src, ri->dstAddr) &&
            tcp->src == ri->dstPort && tcp->dst == ri->srcPort) {
            if ((tcp->flag & TCP_FLAG_RST) != 0) {
                ri->flag = 0;
                ri = NULL;
            } else {
                ri->seq = tcp->seq + GetTcpSegLen(ip, tcp);
                ri->ack = tcp->ack;
                ri->flag |= tcp->flag & TCP_FLAG_ACK;
                ri->id = ip->id - 0x4000;
                if (ri->id == 0) {
                    ri->id = 1;
                }
            }
        }
    }

    if (ri != NULL && ri->id != 0) {
        eh = (ETHHeader*)SendBuf;
        ip = (IPHeader*)(eh + 1);
        eh->type = ETH_IP;
        memmove(eh->dst, ri->addr, sizeof(eh->dst));
        ETHGetMACAddr(eh->src);
        ip->verlen = (4 << 4) | (IP_MIN_HLEN >> 2);
        ip->tos = 0;
        ip->len = IP_MIN_HLEN + TCP_MIN_HLEN + DIGEST_TEXT_SIZE;
        ip->id = ri->id;
        ip->frag = IP_DF;
        ip->ttl = 255;
        ip->proto = IP_PROTO_TCP;
        ip->sum = 0;
        memmove(ip->dst, ri->srcAddr, sizeof(ip->dst));
        memmove(ip->src, ri->dstAddr, sizeof(ip->src));

        tcp = (TCPHeader*)((u8*)ip + IP_HLEN(ip));
        tcp->src = ri->dstPort;
        tcp->dst = ri->srcPort;
        tcp->seq = ri->seq;
        tcp->ack = ri->ack;
        tcp->flag = ((TCP_MIN_HLEN >> 2) << 12) | (ri->flag & 0x3F);
        tcp->win = 0;
        tcp->sum = 0;
        tcp->urg = 0;
        CalcDigest(digest, tcp);
        __IPEncodeToBase64(digest, sizeof(digest), (char*)(tcp + 1));
        ip->sum = CalcIpCheckSum(ip);
        tcp->sum = CalcTcpCheckSum(ip, ip->len);

        ri->flag = 0;
        ETHSendAsync(SendBuf, sizeof(ETHHeader) + IP_MIN_HLEN + TCP_MIN_HLEN + DIGEST_TEXT_SIZE,
                     callback);
    } else if (callback != NULL) {
        callback(ltps);
    }
    return TRUE;
}
