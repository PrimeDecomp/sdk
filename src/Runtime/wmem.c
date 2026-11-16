#include <string.h>
#include <wchar.h>

int wmemcmp(const wchar_t* lhs, const wchar_t* rhs, size_t count) {
  int result = 0;
  while (count--) {
    if ((result = *lhs - *rhs) != 0) {
      break;
    }
    ++lhs;
    ++rhs;
  }
  return result;
}

const wchar_t* wmemchr(const wchar_t* str, wchar_t ch, size_t count) {
  while (count--) {
    if (*str == ch) {
      return str;
    }
    ++str;
  }
  return NULL;
}

wchar_t* wmemcpy(wchar_t* dest, const wchar_t* src, size_t count) {
  return memcpy(dest, src, count * sizeof(wchar_t));
}
