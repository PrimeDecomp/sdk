#ifndef _CTYPE_H_
#define _CTYPE_H_

#include <SDKVersion.h>
#include <locale.h>

#ifdef __cplusplus
extern "C" {
#endif

// eof.h
#define EOF -1L

#if SDK_VERSION <= SDK_VERSION_RVL_2005_11
extern unsigned char __ctype_map[];
extern unsigned char __lower_map[];
extern unsigned char __upper_map[];

#define __control_char 0x01
#define __motion_char 0x02
#define __space_char 0x04
#define __punctuation 0x08
#define __digit 0x10
#define __hex_digit 0x20
#define __lower_case 0x40
#define __upper_case 0x80

#define __letter (__lower_case | __upper_case)
#define __alphanumeric (__letter | __digit)
#define __graphic (__alphanumeric | __punctuation)
#define __printable (__graphic | __space_char)
#define __whitespace (__motion_char | __space_char)
#define __control (__motion_char | __control_char)
#else
// Guessed names
extern const unsigned short __ctype_mapC[256];
extern const unsigned char __lower_mapC[256];
extern const unsigned char __upper_mapC[256];

#define __letter 0x001
#define __blank 0x002
#define __control 0x004
#define __digit 0x008
#define __graphic 0x010
#define __lower_case 0x020
#define __printable 0x040
#define __punctuation 0x080
#define __whitespace 0x100
#define __upper_case 0x200
#define __hex_digit 0x400
#define __alphanumeric (__letter | __digit)
#endif
#define __zero_fill(c) ((int)(unsigned char)(c))

#ifndef _CTYPE_INLINE
#define _CTYPE_INLINE static inline
#endif

// Guessed name
#if SDK_VERSION <= SDK_VERSION_RVL_2005_11
#define __ctype_test(c, mask) (__ctype_map[__zero_fill(c)] & (mask))
#else
#define __ctype_test(c, mask) \
  (((c) < 0 || (c) >= 256) ? 0 : (int)(_current_locale.ctype_cmpt_ptr->ctype_map_ptr[(c)] & (mask)))
#endif

_CTYPE_INLINE
int isalnum(int c) { return __ctype_test(c, __alphanumeric); }
_CTYPE_INLINE
int isalpha(int c) { return __ctype_test(c, __letter); }
_CTYPE_INLINE
int iscntrl(int c) { return __ctype_test(c, __control); }
_CTYPE_INLINE
int isdigit(int c) { return __ctype_test(c, __digit); }
_CTYPE_INLINE
int isgraph(int c) { return __ctype_test(c, __graphic); }
_CTYPE_INLINE
int islower(int c) { return __ctype_test(c, __lower_case); }
_CTYPE_INLINE
int isprint(int c) { return __ctype_test(c, __printable); }
_CTYPE_INLINE
int ispunct(int c) { return __ctype_test(c, __punctuation); }
_CTYPE_INLINE
int isspace(int c) { return __ctype_test(c, __whitespace); }
_CTYPE_INLINE
int isupper(int c) { return __ctype_test(c, __upper_case); }
_CTYPE_INLINE
int isxdigit(int c) { return __ctype_test(c, __hex_digit); }

#undef __ctype_test

#if SDK_VERSION <= SDK_VERSION_RVL_2005_11
_CTYPE_INLINE
int tolower(int c) { return ((c == EOF) ? EOF : ((int)__lower_map[__zero_fill(c)])); }
_CTYPE_INLINE
int toupper(int c) { return ((c == EOF) ? EOF : ((int)__upper_map[__zero_fill(c)])); }
#else
_CTYPE_INLINE
int tolower(int c) {
  return ((c < 0) || (c >= 256)) ? c : (int)_current_locale.ctype_cmpt_ptr->lower_map_ptr[c];
}

_CTYPE_INLINE
int toupper(int c) {
  return ((c < 0) || (c >= 256)) ? c : (int)_current_locale.ctype_cmpt_ptr->upper_map_ptr[c];
}
#endif
_CTYPE_INLINE
int iswblank(int c) { return ((c == (int)L' ') || (c == (int)L'\t')); }

#ifdef __cplusplus
}
#endif

#endif
