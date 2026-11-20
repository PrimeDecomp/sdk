#ifdef SDK_DOLPHIN_IP_2003_09
#include <dolphin/private/ip.h>

typedef DNSInfo DNSCtx;
#else
#define DNSOpen DNSOpen_Reference
#define DNSClose DNSClose_Reference
#include <dolphin/private/ip.h>
#undef DNSOpen
#undef DNSClose

typedef struct DNSCtx {
    UDPInfo udp; // offset 0x0
    IPSocket socket; // offset 0xF0
    u8 unk_F8[0x10]; // offset 0xF8
    OSAlarm alarm; // offset 0x108
    u32 flag; // offset 0x130
    u8 unk_134[0x40C]; // offset 0x134
    u16 id; // offset 0x540
    u8 unk_542[0xE]; // offset 0x542
    s32 ref; // offset 0x550
    OSThreadQueue queueThread; // offset 0x554
} DNSCtx;
#endif

s32 DNSOpen(DNSCtx* info, const u8* dns) {
    s32 result;
    BOOL enabled;
    IPInterface* interface;

    enabled = OSDisableInterrupts();
    result = UDPOpen(&info->udp, NULL, 0);
    if (0 <= result) {
        memset(&info->socket, 0, sizeof(*info) - sizeof(info->udp));
        info->id = (u16)(OSGetTime() / (OS_TIMER_CLOCK / 250000));
        OSCreateAlarm(&info->alarm);
        info->socket.len = IP_SOCKLEN;
        info->socket.family = IP_INET;
        memmove(info->socket.addr, dns, IP_ALEN);
        info->socket.port = 53;
        OSInitThreadQueue(&info->queueThread);
        result = UDPConnect(&info->udp, &info->socket);
        if (result < 0) {
            UDPClose(&info->udp);
        } else {
            interface = IPGetRoute(dns, NULL);
            if (IPIsBroadcastAddr(interface, dns)) {
                info->flag |= 1;
            } else if (IP_CLASSD(dns)) {
                info->flag |= 2;
            }
        }
    }
    OSRestoreInterrupts(enabled);
    return result;
}

s32 DNSClose(DNSCtx* info) {
    s32 result;
    BOOL enabled;

    enabled = OSDisableInterrupts();
    if (info->ref > 0) {
        result = -1;
    } else {
        result = UDPClose(&info->udp);
    }
    if (result >= 0) {
        OSCancelAlarm(&info->alarm);
        OSWakeupThread(&info->queueThread);
    }
    OSRestoreInterrupts(enabled);
    return result;
}
