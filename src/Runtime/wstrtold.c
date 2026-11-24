#include <ansi_fp.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <strtold.h>
#include <wstring_io.h>

enum scan_states {
  start = 0x0001,
  sig_start = 0x0002,
  leading_sig_zeroes = 0x0004,
  int_digit_loop = 0x0008,
  frac_start = 0x0010,
  frac_digit_loop = 0x0020,
  sig_end = 0x0040,
  exp_start = 0x0080,
  leading_exp_digit = 0x0100,
  leading_exp_zeroes = 0x0200,
  exp_digit_loop = 0x0400,
  finished = 0x0800,
  failure = 0x1000
};

#define MAX_SIG_DIG 20
#define FINAL_STATE(state) ((state) & (finished | failure))
#define SUCCESS(state)                                                                            \
  ((state) & (leading_sig_zeroes | int_digit_loop | frac_digit_loop | leading_exp_zeroes |            \
              exp_digit_loop | finished))
#define FETCH() (count++, read_proc(read_arg, 0, __GetChar))
#define UNFETCH(c) read_proc(read_arg, c, __UngetChar)

long double __wcstold(int max_width, wint_t (*read_proc)(void*, wint_t, int), void* read_arg,
                     int* chars_scanned, int* overflow) {
  int scan_state = start;
  int count = 0;
  int spaces = 0;
  wint_t c;
  decimal d = {0, 0, 0, {0, ""}};
  int sig_negative = 0;
  int exp_negative = 0;
  long exp_value = 0;
  int exp_adjust = 0;
  long double result;
  wint_t dot = *(unsigned char*)__lconv.decimal_point;

  *overflow = 0;
  c = FETCH();
  while (count <= max_width && c != EOF && !FINAL_STATE(scan_state)) {
    switch (scan_state) {
    case start:
      if (iswspace(c)) {
        c = FETCH();
        --count;
        ++spaces;
        break;
      }
      if (c == '+') {
        c = FETCH();
      } else if (c == '-') {
        c = FETCH();
        sig_negative = 1;
      }
      scan_state = sig_start;
      break;

    case sig_start:
      if (c == dot) {
        scan_state = frac_start;
        c = FETCH();
        break;
      }
      if (!iswdigit(c)) {
        scan_state = failure;
        break;
      }
      if (c == '0') {
        scan_state = leading_sig_zeroes;
        c = FETCH();
        break;
      }
      scan_state = int_digit_loop;
      break;

    case leading_sig_zeroes:
      if (c == '0') {
        c = FETCH();
        break;
      }
      scan_state = int_digit_loop;
      break;

    case int_digit_loop:
      if (!iswdigit(c)) {
        if (c == dot) {
          scan_state = frac_digit_loop;
          c = FETCH();
        } else {
          scan_state = sig_end;
        }
        break;
      }
      if (d.sig.length < MAX_SIG_DIG) {
        d.sig.text[d.sig.length++] = c;
      } else {
        ++exp_adjust;
      }
      c = FETCH();
      break;

    case frac_start:
      if (!iswdigit(c)) {
        scan_state = failure;
        break;
      }
      scan_state = frac_digit_loop;
      break;

    case frac_digit_loop:
      if (!iswdigit(c)) {
        scan_state = sig_end;
        break;
      }
      if (d.sig.length < MAX_SIG_DIG) {
        if (c != '0' || d.sig.length) {
          d.sig.text[d.sig.length++] = c;
        }
        --exp_adjust;
      }
      c = FETCH();
      break;

    case sig_end:
      if (c == 'E' || c == 'e') {
        scan_state = exp_start;
        c = FETCH();
        break;
      }
      scan_state = finished;
      break;

    case exp_start:
      if (c == '+') {
        c = FETCH();
      } else if (c == '-') {
        c = FETCH();
        exp_negative = 1;
      }
      scan_state = leading_exp_digit;
      break;

    case leading_exp_digit:
      if (!iswdigit(c)) {
        scan_state = failure;
        break;
      }
      if (c == '0') {
        scan_state = leading_exp_zeroes;
        c = FETCH();
        break;
      }
      scan_state = exp_digit_loop;
      break;

    case leading_exp_zeroes:
      if (c == '0') {
        c = FETCH();
        break;
      }
      scan_state = exp_digit_loop;
      break;

    case exp_digit_loop:
      if (!iswdigit(c)) {
        scan_state = finished;
        break;
      }
      exp_value = exp_value * 10 + (c - '0');
      if (exp_value > SHRT_MAX) {
        *overflow = 1;
      }
      c = FETCH();
      break;
    }
  }

  if (!SUCCESS(scan_state)) {
    *chars_scanned = 0;
  } else {
    --count;
    *chars_scanned = count + spaces;
  }
  UNFETCH(c);
  if (exp_negative) {
    exp_value = -exp_value;
  }
  {
    int n = d.sig.length;
    unsigned char* p = &d.sig.text[n];
    while (n-- && *--p == '0') {
      ++exp_adjust;
    }
    d.sig.length = n + 1;
    if (d.sig.length == 0) {
      d.sig.text[d.sig.length++] = '0';
    }
  }
  exp_value += exp_adjust;
  if (exp_value < SHRT_MIN || exp_value > SHRT_MAX) {
    *overflow = 1;
  }
  if (*overflow) {
    if (exp_negative) {
      return 0.0;
    } else {
      return sig_negative ? -HUGE_VAL : HUGE_VAL;
    }
  }
  d.exp = exp_value;
  result = __dec2num(&d);
  if (result != 0.0 && result < LDBL_MIN) {
    *overflow = 1;
    result = 0.0;
  } else if (result > LDBL_MAX) {
    *overflow = 1;
    result = HUGE_VAL;
  }
  if (sig_negative) {
    result = -result;
  }
  return result;
}

double wcstod(const wchar_t* str, wchar_t** end) {
  long double value, abs_value;
  int count, overflow;
  __wInStrCtrl input;
  input.NextChar = str;
  input.NullCharDetected = 0;

  value = __wcstold(INT_MAX, __wStringRead, &input, &count, &overflow);
  if (end != NULL) {
    *end = (wchar_t*)str + count;
  }
  abs_value = fabs(value);
  if (overflow || (value != 0.0 && (abs_value < DBL_MIN || abs_value > DBL_MAX))) {
    errno = ERANGE;
  }
  return value;
}
