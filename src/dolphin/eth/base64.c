#include <dolphin/types.h>
#include <string.h>

// Names are guessed from the operation; this file has no reference source.
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

u8* Base64Encode(const u8* in, int len, u8* out) {
    int rem = len % 3;
    int full = len - rem;
    int i = 0;
    int groups = (full + 2) / 3;
    const char* t = base64_table;

    while (groups-- > 0) {
        out[0] = t[in[i] >> 2];
        out[1] = t[((in[i] & 3) << 4) | (in[i + 1] >> 4)];
        out[2] = t[((in[i + 1] & 15) << 2) | (in[i + 2] >> 6)];
        out[3] = t[in[i + 2] & 63];
        out += 4;
        i += 3;
    }

    switch (rem) {
        case 2:
            out[0] = t[in[full] >> 2];
            out[1] = t[((in[full] & 3) << 4) | (in[full + 1] >> 4)];
            out[2] = t[(in[full + 1] & 15) << 2];
            out[3] = '=';
            out += 4;
            break;
        case 1:
            out[0] = t[in[full] >> 2];
            out[1] = t[(in[full] & 3) << 4];
            out[2] = '=';
            out[3] = '=';
            out += 4;
            break;
    }
    return out;
}

u8* Base64Decode(const char* in, int len, u8* out) {
    const char* t = base64_table;
    int i = 0;
    int a;
    int b;
    int c;
    int d;

    while (i < len) {
        a = strchr(t, in[i]) - t;
        b = strchr(t, in[i + 1]) - t;
        *out++ = (a << 2) | ((b >> 4) & 3);
        if (in[i + 2] == '=') {
            break;
        }
        c = strchr(t, in[i + 2]) - t;
        *out++ = ((b & 15) << 4) | ((c >> 2) & 15);
        if (in[i + 3] == '=') {
            break;
        }
        d = strchr(t, in[i + 3]) - t;
        *out++ = (d & 63) | (c << 6);
        i += 4;
    }
    return out;
}
