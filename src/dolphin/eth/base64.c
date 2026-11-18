#include <dolphin/private/ip.h>
#include <string.h>

static const char Base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

__declspec(weak) char* __IPEncodeToBase64(const void* _src, s32 len, char* dst) {
    const u8* src = _src;
    int i;
    char* p = dst;

    for (i = 0; i < len - len % 3; i += 3) {
        *p++ = Base64[(src[i] >> 2) & 0x3F];
        *p++ = Base64[((src[i] & 3) << 4) | ((src[i + 1] >> 4) & 0xF)];
        *p++ = Base64[((src[i + 1] & 15) << 2) | ((src[i + 2] >> 6) & 3)];
        *p++ = Base64[src[i + 2] & 63];
    }
    i = len - len % 3;
    switch (len % 3) {
    case 2:
        *p++ = Base64[(src[i] >> 2) & 0x3F];
        *p++ = Base64[((src[i] & 3) << 4) | ((src[i + 1] >> 4) & 0xF)];
        *p++ = Base64[(src[i + 1] & 15) << 2];
        *p++ = '=';
        break;
    case 1:
        *p++ = Base64[(src[i] >> 2) & 0x3F];
        *p++ = Base64[(src[i] & 3) << 4];
        *p++ = '=';
        *p++ = '=';
        break;
    }
    return p;
}

__declspec(weak) void* __IPDecodeFromBase64(const char* src, s32 len, void* _dst) {
    u8* dst = _dst;
    int i;
    u8* p = dst;
    int n0;
    int n1;
    int n2;
    int n3;

    for (i = 0; i < len; i += 4) {
        n0 = strchr(Base64, src[i]) - Base64;
        n1 = strchr(Base64, src[i + 1]) - Base64;
        *p++ = (n0 << 2) | ((n1 >> 4) & 3);
        if (src[i + 2] == '=') {
            break;
        }
        n2 = strchr(Base64, src[i + 2]) - Base64;
        *p++ = ((n1 & 15) << 4) | ((n2 >> 2) & 15);
        if (src[i + 3] == '=') {
            break;
        }
        n3 = strchr(Base64, src[i + 3]) - Base64;
        *p++ = ((n2 & 3) << 6) | (n3 & 63);
    }
    return p;
}
