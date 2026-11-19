#ifndef _WCHAR_H_
#define _WCHAR_H_

#include <locale.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned short wint_t;

int fwide(FILE* stream, int mode);
int fputws(const wchar_t* str, FILE* stream);
size_t wcslen(const wchar_t* str);
wchar_t* wcschr(const wchar_t* str, wchar_t ch);
wchar_t* wcscpy(wchar_t* dest, const wchar_t* src);
int wmemcmp(const wchar_t* lhs, const wchar_t* rhs, size_t count);
const wchar_t* wmemchr(const wchar_t* str, wchar_t ch, size_t count);
wchar_t* wmemcpy(wchar_t* dest, const wchar_t* src, size_t count);
unsigned long wcstoul(const wchar_t* str, wchar_t** end, int base);

#ifdef __cplusplus
double wcstod(const wchar_t* str, wchar_t** end);
int swprintf(wchar_t* dest, size_t count, const wchar_t* format, ...);
#endif

static inline int iswalnum(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x009;
}
static inline int iswalpha(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x001;
}
static inline int iswcntrl(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x004;
}
static inline int iswdigit(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x008;
}
static inline int iswlower(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x020;
}
static inline int iswprint(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x040;
}
static inline int iswpunct(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x080;
}
static inline int iswspace(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x100;
}
static inline int iswupper(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x200;
}
static inline int iswxdigit(wint_t ch) {
  return ch >= 256 ? 0 : _current_locale.ctype_cmpt_ptr->wctype_map_ptr[ch] & 0x400;
}

static inline wint_t towlower(wint_t ch) {
  return ch >= 256 ? ch : _current_locale.ctype_cmpt_ptr->wlower_map_ptr[ch];
}

static inline wint_t towupper(wint_t ch) {
  return ch >= 256 ? ch : _current_locale.ctype_cmpt_ptr->wupper_map_ptr[ch];
}

#ifdef __cplusplus
}
#endif

#endif
