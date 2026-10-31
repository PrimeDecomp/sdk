#include <stddef.h>

#include "trk.h"

// Memory is accessed with 32-bit loads and stores only, so each byte access
// is a read-modify-write of the containing word.
static inline u8 TRK_read_byte(const u8 *p) {
  u32 *word = (u32 *)((u32)p & ~3);

  return *word >> ((3 - (p - (u8 *)word)) * 8);
}

static inline void TRK_write_byte(u8 *p, u8 value) {
  u32 *word = (u32 *)((u32)p & ~3);
  u32 shift = (3 - (p - (u8 *)word)) * 8;
  u32 mask = 0xFF << shift;

  *word = (*word & ~mask) | (mask & (value << shift));
}

void *TRK_memcpy(void *dest, const void *src, size_t n) {
  u8 *d = (u8 *)dest;
  const u8 *s = (const u8 *)src;

  while (n != 0) {
    TRK_write_byte(d++, TRK_read_byte(s++));
    n--;
  }

  return dest;
}

void *TRK_fill_mem(void *dest, int val, size_t n) {
  u8 *d = (u8 *)dest;
  u8 byte = val;

  while (n != 0) {
    TRK_write_byte(d++, byte);
    n--;
  }

  return dest;
}
