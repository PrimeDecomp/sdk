#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <strtoul.h>
#include <wstring_io.h>

enum scan_states {
  start = 0x01,
  check_for_zero = 0x02,
  leading_zero = 0x04,
  need_digit = 0x08,
  digit_loop = 0x10,
  finished = 0x20,
  failure = 0x40
};

#define FETCH() (count++, read_proc(read_arg, 0, __GetChar))
#define UNFETCH(c) read_proc(read_arg, c, __UngetChar)

unsigned long __wcstoul(int initial_base, int max_width,
                       wint_t (*read_proc)(void*, wint_t, int), void* read_arg,
                       int* chars_scanned, int* negative, int* overflow) {
  int scan_state = start;
  int count = 0;
  unsigned long value = 0;
  unsigned long value_max = 0;
  wint_t c;
  int base = initial_base;

  *negative = *overflow = 0;
  if (base < 0 || base == 1 || base > 36 || max_width < 1) {
    scan_state = failure;
  } else {
    c = FETCH();
  }
  if (base != 0) {
    value_max = ULONG_MAX / base;
  }

  while (count <= max_width && c != EOF && !(scan_state & (finished | failure))) {
    switch (scan_state) {
    case start:
      if (iswspace(c)) {
        c = FETCH();
        break;
      }
      if (c == '+') {
        c = FETCH();
      } else if (c == '-') {
        c = FETCH();
        *negative = 1;
      }
      scan_state = check_for_zero;
      break;

    case check_for_zero:
      if ((base == 0 || base == 16) && c == '0') {
        scan_state = leading_zero;
        c = FETCH();
        break;
      }
      scan_state = need_digit;
      break;

    case leading_zero:
      if (c == 'X' || c == 'x') {
        base = 16;
        scan_state = need_digit;
        c = FETCH();
        break;
      }
      if (base == 0) {
        base = 8;
      }
      scan_state = digit_loop;
      break;

    case need_digit:
    case digit_loop:
      if (base == 0) {
        base = 10;
      }
      if (value_max == 0) {
        value_max = ULONG_MAX / base;
      }
      if (iswdigit(c)) {
        if ((c -= '0') >= base) {
          if (scan_state == digit_loop) {
            scan_state = finished;
          } else {
            scan_state = failure;
          }
          c += '0';
          break;
        }
      } else if (!iswalpha(c) || toupper(c) - 'A' + 10 >= base) {
        if (scan_state == digit_loop) {
          scan_state = finished;
        } else {
          scan_state = failure;
        }
        break;
      } else {
        c = towupper(c) - 'A' + 10;
      }
      if (value > value_max) {
        *overflow = 1;
      }
      value *= base;
      if (c > ULONG_MAX - value) {
        *overflow = 1;
      }
      value += c;
      scan_state = digit_loop;
      c = FETCH();
      break;
    }
  }

  if (!(scan_state & (leading_zero | digit_loop | finished))) {
    count = value = 0;
  } else {
    --count;
  }
  *chars_scanned = count;
  UNFETCH(c);
  return value;
}

unsigned long wcstoul(const wchar_t* str, wchar_t** end, int base) {
  unsigned long value;
  int count, negative, overflow;
  __wInStrCtrl input;
  input.NextChar = str;
  input.NullCharDetected = 0;

  value = __wcstoul(base, INT_MAX, __wStringRead, &input, &count, &negative, &overflow);
  if (end != NULL) {
    *end = (wchar_t*)str + count;
  }
  if (overflow) {
    value = ULONG_MAX;
    errno = ERANGE;
  } else if (negative) {
    value = -value;
  }
  return value;
}
