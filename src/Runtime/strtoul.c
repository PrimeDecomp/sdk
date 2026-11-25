#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <strtoul.h>

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

unsigned long __strtoul(int base, int max_width,
                       int (*read_proc)(void*, int, int), void* read_arg,
                       int* chars_scanned, int* negative, int* overflow) {
  int scan_state = start;
  int count = 0;
  int spaces = 0;
  unsigned long value = 0;
  unsigned long value_max = 0;
  int c;

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
      if (isspace(c)) {
        c = FETCH();
        --count;
        ++spaces;
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
      if (!value_max) {
        value_max = ULONG_MAX / base;
      }
      if (isdigit(c)) {
        if ((c -= '0') >= base) {
          if (scan_state == digit_loop) {
            scan_state = finished;
          } else {
            scan_state = failure;
          }
          c += '0';
          break;
        }
      } else if (!isalpha(c) || toupper(c) - 'A' + 10 >= base) {
        if (scan_state == digit_loop) {
          scan_state = finished;
        } else {
          scan_state = failure;
        }
        break;
      } else {
        c = toupper(c) - 'A' + 10;
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
    value = *chars_scanned = 0;
  } else {
    --count;
    *chars_scanned = count + spaces;
  }
  UNFETCH(c);
  return value;
}

unsigned long long __strtoull(int base, int max_width,
                             int (*read_proc)(void*, int, int), void* read_arg,
                             int* chars_scanned, int* negative, int* overflow) {
  int scan_state = start;
  int count = 0;
  int spaces = 0;
  unsigned long long value = 0;
  unsigned long long value_max = 0;
  int c;

  *negative = *overflow = 0;
  if (base < 0 || base == 1 || base > 36 || max_width < 1) {
    scan_state = failure;
  } else {
    c = FETCH();
  }
  if (base != 0) {
    value_max = (~0ULL) / base;
  }

  while (count <= max_width && c != EOF && !(scan_state & (finished | failure))) {
    switch (scan_state) {
    case start:
      if (isspace(c)) {
        c = FETCH();
        --count;
        ++spaces;
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
      if (!value_max) {
        value_max = (~0ULL) / base;
      }
      if (isdigit(c)) {
        if ((c -= '0') >= base) {
          if (scan_state == digit_loop) {
            scan_state = finished;
          } else {
            scan_state = failure;
          }
          c += '0';
          break;
        }
      } else if (!isalpha(c) || toupper(c) - 'A' + 10 >= base) {
        if (scan_state == digit_loop) {
          scan_state = finished;
        } else {
          scan_state = failure;
        }
        break;
      } else {
        c = toupper(c) - 'A' + 10;
      }
      if (value > value_max) {
        *overflow = 1;
      }
      value *= base;
      if (c > (~0ULL) - value) {
        *overflow = 1;
      }
      value += c;
      scan_state = digit_loop;
      c = FETCH();
      break;
    }
  }

  if (!(scan_state & (leading_zero | digit_loop | finished))) {
    value = *chars_scanned = 0;
  } else {
    --count;
    *chars_scanned = count + spaces;
  }
  UNFETCH(c);
  return value;
}

unsigned long strtoul(const char* str, char** end, int base) {
  unsigned long value;
  int count, negative, overflow;
  __InStrCtrl input;
  input.NextChar = (char*)str;
  input.NullCharDetected = 0;

  value = __strtoul(base, INT_MAX, __StringRead, &input, &count, &negative, &overflow);
  if (end != NULL) {
    *end = (char*)str + count;
  }
  if (overflow) {
    value = ULONG_MAX;
    errno = ERANGE;
  } else if (negative) {
    value = -value;
  }
  return value;
}

int atoi(const char* str) {
  unsigned long value;
  long result;
  int overflow, negative, count;
  __InStrCtrl input;
  input.NextChar = (char*)str;
  input.NullCharDetected = 0;

  value = __strtoul(10, INT_MAX, __StringRead, &input, &count, &negative, &overflow);
  if (overflow || (!negative && value > LONG_MAX) ||
      (negative && value > (unsigned long)LONG_MAX + 1)) {
    result = negative ? LONG_MIN : LONG_MAX;
    errno = ERANGE;
  } else {
    result = negative ? -value : value;
  }
  return result;
}
