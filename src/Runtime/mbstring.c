#include <SDKVersion.h>
#include "errno.h"
#include "stdio.h"
#include "string.h"

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
#pragma ANSI_strict off

#ifndef __WINT_TYPE__
#define __WINT_TYPE__ unsigned int
#endif

typedef struct {
  int __count;
  union {
    __WINT_TYPE__ __wch;
    char __wchb[4];
  } __value; /* Value so far.  */
} mbstate_t;

static int is_utf8_complete(const char* s, size_t n);
static int is_utf8_complete(const char* s, size_t n) {
  if (n <= 0) /* 0 or fewer characters do not form a valid multibyte character */
    return (-1);

  if (*s == 0x00)
    return (0);

  if ((*s & 0x80) == 0x00)
    return (1);
  else if ((*s & 0xe0) == 0xc0) /* need 2 bytes */
    if (n >= 2)
      if (((*(s + 1) & 0x80) == 0x80))
        return (2);
      else
        return (-1);
    else
      return (-2);

  else if ((*s & 0xf0) == 0xe0)
    if (n >= 3)
      if (((*(s + 1) & 0x80) == 0x80) && ((*(s + 2) & 0x80) == 0x80))
        return (3);
      else
        return (-1);
    else if (((n == 2) && ((*(s + 1) & 0x80) == 0x80)) || (n == 1))
      return (-2);
    else
      return (-1);
  else
    return (-1);
}

#pragma dont_inline on
static int unicode_to_UTF8(char* s, wchar_t wchar) {
  int number_of_bytes;
  wchar_t wide_char;
  char* target_ptr;
  char first_byte_mark[4] = {0x00, 0x00, 0xc0, 0xe0};

  if (!s) {
    return 0;
  }

  wide_char = wchar;
  if (wide_char < 0x0080)
    number_of_bytes = 1;
  else if (wide_char < 0x0800)
    number_of_bytes = 2;
  else
    number_of_bytes = 3;

  target_ptr = s + number_of_bytes;

  switch (number_of_bytes) {
  case 3:
    *--target_ptr = (wide_char & 0x003f) | 0x80;
    wide_char >>= 6;
  case 2:
    *--target_ptr = (wide_char & 0x003f) | 0x80;
    wide_char >>= 6;
  case 1:
    *--target_ptr = wide_char | first_byte_mark[number_of_bytes];
  }

  return (number_of_bytes);
}
#pragma dont_inline reset

int wctomb(char* s, wchar_t wchar) { return (unicode_to_UTF8(s, wchar)); }
#else
#include <locale.h>

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
static int is_utf8_complete(const char* s, size_t n) {
  if (n <= 0) /* 0 or fewer characters do not form a valid multibyte character */
    return (-1);

  if (*s == 0x00)
    return (0);

  if ((*s & 0x80) == 0x00)
    return (1);
  else if ((*s & 0xe0) == 0xc0) /* need 2 bytes */
    if (n >= 2)
      if (((*(s + 1) & 0x80) == 0x80))
        return (2);
      else
        return (-1);
    else
      return (-2);

  else if ((*s & 0xf0) == 0xe0)
    if (n >= 3)
      if (((*(s + 1) & 0x80) == 0x80) && ((*(s + 2) & 0x80) == 0x80))
        return (3);
      else
        return (-1);
    else if (((n == 2) && ((*(s + 1) & 0x80) == 0x80)) || (n == 1))
      return (-2);
    else
      return (-1);
  else
    return (-1);
}
#endif

#if SDK_VERSION == SDK_VERSION_DOLPHIN_2004_04
static int utf8_to_unicode(wchar_t* dst, const char* src, size_t n) {
  int bytes, expected;
  char* p;
  wchar_t wide_char = 0;

  if (!src)
    return 0;

  if (n <= 0)
    return -1;

  bytes = is_utf8_complete(src, n);
  if (bytes < 0)
    return -1;

  p = (char*)src;
  switch (bytes) {
  case 3:
    wide_char |= (*p++ & 0x0f);
    wide_char <<= 6;
  case 2:
    wide_char |= (*p++ & 0x3f);
    wide_char <<= 6;
  case 1:
    wide_char |= (*p++ & 0x7f);
  }

  if (wide_char == 0)
    expected = 0;
  else if (wide_char < 0x0080)
    expected = 1;
  else if (wide_char < 0x0800)
    expected = 2;
  else
    expected = 3;

  if (expected != bytes)
    return -1;

  if (dst)
    *dst = wide_char;

  return bytes;
}
#endif

int mbtowc(wchar_t *dst, const char *src, size_t n) {
#if SDK_VERSION == SDK_VERSION_DOLPHIN_2004_04
  return utf8_to_unicode(dst, src, n);
#else
  return _current_locale.ctype_cmpt_ptr->decode_mb(dst, src, n);
#endif
}

// Guessed name
int __mbtowc_noconv(wchar_t *dst, const char *src, size_t n) {
  if (!src)
    return 0;

  if (!n)
    return -1;

  if (dst)
    *dst = (unsigned char)*src;

  if (!*src)
    return 0;

  return 1;
}

// Guessed name
int __wctomb_noconv(char *dst, wchar_t ch) {
  if (!dst)
    return 0;

  *dst = ch;
  return 1;
}

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04 && SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
#pragma ANSI_strict off

#ifndef __WINT_TYPE__
#define __WINT_TYPE__ unsigned int
#endif

typedef struct {
  int __count;
  union {
    __WINT_TYPE__ __wch;
    char __wchb[4];
  } __value; /* Value so far.  */
} mbstate_t;

static int unicode_to_UTF8(char* s, wchar_t wchar) {
  int number_of_bytes;
  wchar_t wide_char;
  char* target_ptr;
  char first_byte_mark[4] = {0x00, 0x00, 0xc0, 0xe0};

  if (!s) {
    return 0;
  }

  wide_char = wchar;
  if (wide_char < 0x0080)
    number_of_bytes = 1;
  else if (wide_char < 0x0800)
    number_of_bytes = 2;
  else
    number_of_bytes = 3;

  target_ptr = s + number_of_bytes;

  switch (number_of_bytes) {
  case 3:
    *--target_ptr = (wide_char & 0x003f) | 0x80;
    wide_char >>= 6;
  case 2:
    *--target_ptr = (wide_char & 0x003f) | 0x80;
    wide_char >>= 6;
  case 1:
    *--target_ptr = wide_char | first_byte_mark[number_of_bytes];
  }

  return (number_of_bytes);
}

int wctomb(char* s, wchar_t wchar) { return (unicode_to_UTF8(s, wchar)); }

#else
static inline int wctomb(char *dst, wchar_t ch) {
  return _current_locale.ctype_cmpt_ptr->encode_wc(dst, ch);
}
#endif
#endif

#if SDK_VERSION == SDK_VERSION_DOLPHIN_2004_04
size_t mbstowcs(wchar_t* dest, const char* src, size_t count) {
  int result;
  const char* p;
  int written;
  size_t remaining;

  remaining = strlen(src);
  if (dest) {
    p = src;
    for (written = 0; written < count; ++written) {
      if (*p) {
        result = mbtowc(dest++, p, remaining);
        if (result > 0) {
          p += result;
          remaining -= result;
        } else {
          return (size_t)-1;
        }
      } else {
        *dest = 0;
        break;
      }
    }
  } else {
    written = 0;
  }
  return written;
}
#endif

size_t wcstombs(char* s, const wchar_t* pwcs, size_t n) {
  int chars_written = 0;
  int result;
  char temp[3];
  wchar_t* source;

  if (!s || !pwcs)
    return (0);

  source = (wchar_t*)pwcs;
  while (chars_written <= n) {
    if (!*source) {
      *(s + chars_written) = '\0';
      break;
    } else {
      result = wctomb(temp, *source++);
      if ((chars_written + result) <= n) {
        strncpy(s + chars_written, temp, result);
        chars_written += result;
      } else
        break;
    }
  }

  return (chars_written);
}
