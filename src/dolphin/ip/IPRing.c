#include <dolphin/private/ip.h>

#ifdef DEBUG
#define IFFifoMemset(p, c, n) memset(p, c, n)
#else
#define IFFifoMemset(p, c, n)
#endif

// Range: 0x0 -> 0x98
void IFFifoInit(IFFifo* fifo /* r29 */, void* buff /* r30 */, s32 size /* r31 */) {
    ASSERTLINE(57, size == 0 || (buff && 0 < size));
    fifo->size = size;
    fifo->buff = buff;
    fifo->head = buff;
    fifo->used = 0;

#ifdef DEBUG
    if (buff && size > 0) {
        IFFifoMemset(buff, 0xFF, size);
    }
#endif
}

// Range: 0x98 -> 0x24C
static void* FifoAlloc(IFFifo* fifo /* r31 */, s32 len /* r28 */) {
    // Local variables
    s32 free; // r26
    u8* tail; // r29
    u8* end; // r27

    end = fifo->buff + fifo->size;
    ASSERTLINE(75, 0 < len);
    ASSERTLINE(76, 0 <= fifo->used && fifo->used <= fifo->size);
    ASSERTLINE(77, fifo->buff <= fifo->head && fifo->head <= end);

    if (fifo->size - fifo->used < len) {
        return NULL;
    }

    tail = fifo->head + fifo->used;
    if (end <= tail) {
        tail -= fifo->size;
    }

    if (fifo->head == tail) {
        fifo->head = fifo->buff;
        fifo->used = len;
        return fifo->buff;
    } else if (fifo->head < tail) {
        free = (s32)(end - tail);
        if (len <= free) {
            fifo->used += len;
            return tail;
        } else if (len <= (s32)(fifo->head - fifo->buff)) {
            fifo->used += free + len;
            return fifo->buff;
        }
    } else {
        ASSERTLINE(118, fifo->head - tail == fifo->size - fifo->used);
        fifo->used += len;
        return tail;
    }

    return NULL;
}

// Range: 0x24C -> 0x2D0
void * IFFifoAlloc(IFFifo* fifo /* r30 */, s32 len /* r29 */) {
    // Local variables
    void* ptr; // r31

    ptr = FifoAlloc(fifo, len);
    if (ptr) {
        IFFifoMemset(ptr, 0xA3, len);
    }

    ASSERTLINE(137, fifo->used <= fifo->size);
    return ptr;
}

// Range: 0x2D0 -> 0x648
BOOL IFFifoFree(IFFifo* fifo /* r31 */, void* ptr /* r1+0xC */, s32 len /* r26 */) {
    // Local variables
    u8* p; // r29
    u8* end; // r27
    u8* head; // r30
    u8* tail; // r28

    p = (u8*)ptr;
    end = fifo->buff + fifo->size;
    ASSERTLINE(148, 0 <= fifo->used && fifo->used <= fifo->size);
    ASSERTLINE(149, fifo->buff <= fifo->head && fifo->head < end);

    if (len <= 0 || fifo->used < len || p == NULL || p < fifo->buff || end <= p) {
        return FALSE;
    }

    tail = fifo->head + fifo->used;
    if (end <= tail) {
        tail -= fifo->size;
    }

    head = p + len;
    if (end <= head) {
        head -= fifo->size;
    }

    if (head < fifo->buff || end <= head) {
        return FALSE;
    }

    if (fifo->head == tail) {
        if (fifo->head < head) {
            if (head <= p || p < fifo->head) {
                return FALSE;
            }

            IFFifoMemset(fifo->head, -0xA4, (size_t)(head - fifo->head));
        } else {
            if (head <= p && p < fifo->head) {
                return FALSE;
            }

            if (head == tail) {
                IFFifoMemset(p, -0xA4, len);
                fifo->used -= len;
                return TRUE;
            }

            IFFifoMemset(fifo->head, -0xA4, (size_t)(end - fifo->head));
            IFFifoMemset(fifo->buff, -0xA4, (size_t)(head - fifo->buff));
        }
    } else if (fifo->head < tail) {
        if (head <= p || p < fifo->head || tail < head) {
            return FALSE;
        }

        if (head == tail) {
            IFFifoMemset(p, -0xA4, len);
            fifo->used -= len;
            return TRUE;
        }

        IFFifoMemset(fifo->head, -0xA4, (size_t)(head - fifo->head));
    } else if (fifo->head < head) {
        if (head <= p || p < fifo->head) {
            return FALSE;
        }

        IFFifoMemset(fifo->head, -0xA4, (size_t)(head - fifo->head));
    } else {
        if (tail < head) {
            return FALSE;
        }

        if (head <= p && p < fifo->head) {
            return FALSE;
        }

        if (head == tail) {
            IFFifoMemset(p, -0xA4, len);
            fifo->used -= len;
            return TRUE;
        }

        IFFifoMemset(fifo->head, -0xA4, (size_t)(end - fifo->head));
        IFFifoMemset(fifo->buff, -0xA4, (size_t)(head - fifo->buff));
    }

    if (head <= tail) {
        fifo->used = (s32)tail - (s32)head;
    } else {
        fifo->used = ((s32)end - (s32)head) + ((s32)tail - (s32)fifo->buff);
    }

    ASSERTLINE(285, fifo->used < fifo->size);
    fifo->head = head;
    return TRUE;
}

// Range: 0x648 -> 0x748
void IFDump(void* ptr /* r28 */, s32 len /* r27 */) {
    // Local variables
    int j; // r30
    int i; // r31
    int n; // r29

    for (j = 0; j < len; j += 16) {
        n = len - j;
        if (n > 16) {
            n = 16;
        }

        OSReport("%08x: ", j);
        for (i = 0; i < n; i++) {
            OSReport("%02x ", ((u8*)ptr)[j + i]);
        }

        for (i; i < 16; i++) {
            OSReport("   ");
        }

        OSReport("  ");
        for (i = 0; i < n; i++) {
            OSReport("%c", isprint(((u8*)ptr)[j + i]) ? ((u8*)ptr)[j + i] : '.');
        }

        OSReport("\n");
    }
}


// Range: 0x0 -> 0x10C
u8* IFRingIn(u8* buf /* r23 */, s32 size /* r24 */, u8* head /* r30 */, s32 used /* r22 */, const u8* data /* r26 */, s32 len /* r28 */) {
    // Local variables
    u8* end; // r25
    u8* tail; // r31
    s32 free; // r27

    ASSERTLINE(70, used + len <= size);
    end = buf + size;
    ASSERTLINE(72, buf <= head && head < end);
    tail = head + used;
    if (end <= tail) {
        tail -= size;
    }

    if (head <= tail) {
        free = (s32)end - (s32)tail;
        if (len <= free) {
            memmove(tail, data, len);
            return head;
        } else {
            memmove(tail, data, free);
            data += free;
            len -= free;
            memmove(buf, data, len);
            return head;
        }
    } else {
        memmove(tail, data, len);
        return head;
    }
}

// Range: 0x10C -> 0x238
u8* IFRingOut(u8* buf /* r25 */, s32 size /* r1+0xC */, u8* head /* r31 */, s32 used /* r1+0x14 */, u8* data /* r26 */, s32 len /* r29 */) {
    // Local variables
    u8* end; // r28
    s32 front; // r27

    ASSERTLINE(131, len <= used);
    end = buf + size;
    ASSERTLINE(133, buf <= head && head < end);

    if (head + len < end) {
        memmove(data, head, len);
        head += len;
    } else {
        front = (s32)end - (s32)head;
        ASSERTLINE(159, front <= len);
        memmove(data, head, front);
        data += front;
        len -= front;
        head = buf;
        memmove(data, head, len);
        head += len;
    }

    ASSERTLINE(167, buf <= head && head < end);
    return head;
}

// Range: 0x238 -> 0x328
int IFRingGet(u8* buf /* r25 */, s32 size /* r24 */, u8* head /* r31 */, s32 used /* r1+0x14 */, IFVec* vec /* r30 */, s32 len /* r27 */) {
    // Local variables
    u8* end; // r28
    s32 front; // r26

    ASSERTLINE(193, len <= used);
    end = buf + size;
    if (end <= head) {
        head -= size;
    }
    ASSERTLINE(200, buf <= head && head < end);

    if (head + len <= end) {
        vec->data = head;
        vec->len = len;
        return 1; // one entry
    } else {
        front = (s32)end - (s32)head;
        ASSERTLINE(227, front < len);
        vec->data = head;
        vec->len = front;
        vec++;
        vec->data = buf;
        vec->len = len - front;
        return 2; // two entries
    }
}

// Range: 0x328 -> 0x3FC
u8* IFRingPut(u8* buf /* r27 */, s32 size /* r1+0xC */, u8* head /* r31 */, s32 used /* r1+0x14 */, s32 len /* r28 */) {
    // Local variables
    u8* end; // r29

    ASSERTLINE(258, len <= used);
    end = buf + size;
    ASSERTLINE(260, buf <= head && head < end);
    if (head + len < end) {
        head += len;
    } else {
        head = (buf + len) - (end - head);
    }

    ASSERTLINE(285, buf <= head && head < end);
    return head;
}

// Range: 0x3FC -> 0x650
static s32 MargeBlock(u8* ptr /* r23 */, s32 len /* r24 */, IFBlock* blockTable /* r25 */, s32 maxblock /* r20 */, s32 size /* r21 */, u8* tail /* r27 */) {
    // Local variables
    IFBlock* block; // r31
    IFBlock* end; // r28
    s32 pl; // r26
    s32 pr; // r30
    s32 pb; // r29

    ASSERTLINE(318, 1 < maxblock && blockTable);
    ASSERTLINE(319, 0 <= len);

    pl = tail <= ptr ? (s32)ptr - (s32)tail : (s32)ptr + size - (s32)tail;
    pr = pl + len;
    end = blockTable + maxblock;
    if (tail == ptr) {
        block = blockTable;

        while (block < end && block->ptr) {
            pb = tail <= block->ptr ? (s32)block->ptr - (s32)tail : (s32)block->ptr + size - (s32)tail;
            if (pb <= pr) {
                pr = pb + block->len > pr ? pb + block->len : pr;
                len = pr - pl;
                memmove(block, block + 1, (s32)end - (s32)(block + 1));
                memset(end - 1, 0, sizeof(IFBlock));
            } else {
                block++;
            }
        }

        return len;
    } else {
        block = blockTable;
        while (block < end && block->ptr) {
            pb = tail <= block->ptr ? (s32)block->ptr - (s32)tail : (s32)block->ptr + size - (s32)tail;
            if (pl <= pb + block->len && pb <= pr) {
                pr = pb + block->len > pr ? pb + block->len : pr;

                if (pb < pl) {
                    pl = pb;
                    ptr = block->ptr;
                }

                len = pr - pl;
                memmove(block, block + 1, (s32)end - (s32)(block + 1));
                memset(end - 1, 0, sizeof(IFBlock));
            } else {
                block++;
            }
        }

        if (block < end) {
            ASSERTLINE(367, block->ptr == NULL);
            block->ptr = ptr;
            block->len = len;
        } else {
            memmove(blockTable, blockTable + 1, (s32)end - (s32)(blockTable + 1));
            block = end - 1;
            block->ptr = ptr;
            block->len = len;
        }

        return 0;
    }
}

// Range: 0x650 -> 0x7D4
u8* IFRingInEx(u8* buf /* r21 */, s32 size /* r27 */, u8* head /* r28 */, s32 used /* r19 */, s32 offset /* r22 */, const u8* data /* r23 */, s32 * adv /* r20 */, IFBlock* blockTable /* r1+0x24 */, s32 maxblock /* r1+0x68 */) {
    // Local variables
    u8* end; // r26
    u8* tail; // r25
    u8* ptr; // r31
    s32 len; // r29
    s32 free; // r24

    len = *adv;
    if (len == 0) {
        return head;
    }

    ASSERTLINE(397, 0 <= offset);
    ASSERTLINE(398, used + offset + len <= size);
    end = buf + size;
    ASSERTLINE(400, buf <= head && head < end);
    tail = head + used;
    if (end <= tail) {
        tail -= size;
    }

    ASSERTLINE(407, offset < size);
    ptr = tail + offset;
    if (end <= ptr) {
        ptr -= size;
    }

    if (head <= ptr) {
        free = (s32)end - (s32)ptr;
        if (len <= free) {
            memmove(ptr, data, len);
        } else {
            memmove(ptr, data, free);
            memmove(buf, data + free, len - free);
        }
    } else {
        memmove(ptr, data, len);
    }

    *adv = MargeBlock(ptr, len, blockTable, maxblock, size, tail);
    return head;
}
