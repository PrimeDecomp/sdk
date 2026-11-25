#include <ansi_fp.h>
#include <ctype.h>
#include <float.h>
#include <locale.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>


enum justification_options { left_justification, right_justification, zero_fill };
enum sign_options { only_minus, sign_always, space_holder };
enum argument_options {
  normal_argument,
  char_argument,
  short_argument,
  long_argument,
  long_long_argument,
  long_double_argument,
  wchar_argument
};

// Guessed name
typedef struct {
  u8 justification_options;
  u8 sign_options;
  u8 precision_specified;
  u8 alternate_form;
  u8 argument_options;
  wchar_t conversion_char;
  int field_width;
  int precision;
} print_format;

// Guessed name
typedef struct {
  wchar_t* dest;
  size_t capacity;
  size_t written;
} __wOutStrCtrl;

static const wchar_t* parse_wformat(const wchar_t* format_string, va_list* arg, print_format* format) {
  print_format f;
  const wchar_t* s = format_string;
  wchar_t c;
  int flag_found;
  f.justification_options = right_justification;
  f.sign_options = only_minus;
  f.precision_specified = 0;
  f.alternate_form = 0;
  f.argument_options = normal_argument;
  f.field_width = 0;
  f.precision = 0;

  if ((c = *++s) == '%') {
    f.conversion_char = c;
    *format = f;
    return ((const wchar_t*)s + 1);
  }

  for (;;) {
    flag_found = 1;

    switch (c) {
    case '-':
      f.justification_options = left_justification;
      break;
    case '+':
      f.sign_options = sign_always;
      break;
    case ' ':
      if (f.sign_options != sign_always) {
        f.sign_options = space_holder;
      }
      break;
    case '#':
      f.alternate_form = 1;
      break;
    case '0':
      if (f.justification_options != left_justification) {
        f.justification_options = zero_fill;
      }
      break;
    default:
      flag_found = 0;
      break;
    }

    if (flag_found) {
      c = *++s;
    } else {
      break;
    }
  }

  if (c == '*') {
    if ((f.field_width = va_arg(*arg, int)) < 0) {
      f.justification_options = left_justification;
      f.field_width = -f.field_width;
    }

    c = *++s;
  } else {
    while (iswdigit(c)) {
      f.field_width = (f.field_width * 10) + (c - '0');
      c = *++s;
    }
  }

  if (f.field_width > 509) {
    f.conversion_char = 0xFFFF;
    *format = f;
    return ((const wchar_t*)s + 1);
  }

  if (c == '.') {
    f.precision_specified = 1;

    if ((c = *++s) == '*') {
      if ((f.precision = va_arg(*arg, int)) < 0) {
        f.precision_specified = 0;
      }

      c = *++s;
    } else {
      while (iswdigit(c)) {
        f.precision = (f.precision * 10) + (c - '0');
        c = *++s;
      }
    }
  }

  flag_found = 1;

  switch (c) {
  case 'h':
    f.argument_options = short_argument;

    if (s[1] == 'h') {
      f.argument_options = char_argument;
      c = *++s;
    }

    break;

  case 'l':
    f.argument_options = long_argument;

    if (s[1] == 'l') {
      f.argument_options = long_long_argument;
      c = *++s;
    }

    break;

  case 'L':
    f.argument_options = long_double_argument;
    break;
  default:
    flag_found = 0;
    break;
  }

  if (flag_found) {
    c = *++s;
  }

  f.conversion_char = c;

  switch (c) {
  case 'd':
  case 'i':
  case 'u':
  case 'o':
  case 'x':
  case 'X':
    if (f.argument_options == long_double_argument) {
      f.argument_options = long_long_argument;
    }

    if (!f.precision_specified) {
      f.precision = 1;
    } else if (f.justification_options == zero_fill) {
      f.justification_options = right_justification;
    }
    break;

  case 'f':
  case 'F':
    if (f.argument_options == short_argument || f.argument_options == long_long_argument) {
      f.conversion_char = 0xFFFF;
      break;
    }

    if (!f.precision_specified) {
      f.precision = 6;
    }
    break;


  case 'g':
  case 'G':
    if (!f.precision) {
      f.precision = 1;
    }

  case 'e':
  case 'E':
    if (f.argument_options == short_argument || f.argument_options == long_long_argument ||
        f.argument_options == char_argument) {
      f.conversion_char = 0xFFFF;
      break;
    }

    if (!f.precision_specified) {
      f.precision = 6;
    }
    break;

  case 'p':
    f.argument_options = long_argument;
    f.alternate_form = 1;
    f.conversion_char = 'x';
    f.precision = 8;
    break;

  case 'c':
    if (f.argument_options == long_argument) {
      f.argument_options = wchar_argument;
    } else {
      if (f.precision_specified || f.argument_options != normal_argument) {
        f.conversion_char = 0xFFFF;
      }
    }

    break;

  case 's':
    if (f.argument_options == long_argument) {
      f.argument_options = wchar_argument;
    } else {
      if (f.argument_options != normal_argument) {
        f.conversion_char = 0xFFFF;
      }
    }

    break;

  case 'n':
    if (f.argument_options == long_double_argument) {
      f.argument_options = long_long_argument;
    }

    break;

  default:
    f.conversion_char = 0xFFFF;
    break;
  }

  *format = f;
  return ((const wchar_t*)s + 1);
}

static wchar_t* long2wstr(s32 num, wchar_t* buff, print_format format) {
  u32 unsigned_num, base;
  wchar_t* p;
  int n, digits;
  int precision;
  int minus = 0;
  unsigned_num = num;
  minus = 0;

  p = buff;
  *--p = 0;
  digits = 0;

  if (!num && !format.precision && !(format.alternate_form && format.conversion_char == 'o')) {
    return p;
  }

  switch (format.conversion_char) {
  case 'd':
  case 'i':
    base = 10;

    if (num < 0) {
      unsigned_num = -unsigned_num;
      minus = 1;
    }
    break;

  case 'o':
    base = 8;
    format.sign_options = only_minus;
    break;

  case 'u':
    base = 10;
    format.sign_options = only_minus;
    break;

  case 'x':
  case 'X':
    base = 16;
    format.sign_options = only_minus;
    break;
  }

  do {
    n = unsigned_num % base;
    unsigned_num /= base;

    if (n < 10) {
      n += '0';
    } else {
      n -= 10;

      if (format.conversion_char == 'x') {
        n += 'a';
      } else {
        n += 'A';
      }
    }

    *--p = n;
    ++digits;
  } while (unsigned_num != 0);

  if (base == 8 && format.alternate_form && *p != '0') {
    *--p = '0';
    ++digits;
  }

  if (format.justification_options == zero_fill) {
    format.precision = format.field_width;

    if (minus || format.sign_options != only_minus)
      --format.precision;

    if (base == 16 && format.alternate_form)
      format.precision -= 2;
  }

  precision = format.precision;

  if (buff - p + precision > 509)
    return (0);

  while (digits < precision) {
    *--p = '0';
    ++digits;
  }

  if (base == 16 && format.alternate_form) {
    *--p = format.conversion_char;
    *--p = '0';
  }

  if (minus) {
    *--p = '-';
  } else if (format.sign_options == sign_always) {
    *--p = '+';
  } else if (format.sign_options == space_holder) {
    *--p = ' ';
  }

  return p;
}

static wchar_t* longlong2wstr(s64 num, wchar_t* pBuf, print_format fmt) {
  u64 unsigned_num, base;
  wchar_t* p;
  int n, digits;
  int precision;
  int minus = 0;
  unsigned_num = num;
  minus = 0;
  p = pBuf;
  *--p = 0;
  digits = 0;

  if (!num && !fmt.precision && !(fmt.alternate_form && fmt.conversion_char == 'o')) {
    return p;
  }

  switch (fmt.conversion_char) {
  case 'd':
  case 'i':
    base = 10;

    if (num < 0) {
      unsigned_num = -unsigned_num;
      minus = 1;
    }
    break;
  case 'o':
    base = 8;
    fmt.sign_options = only_minus;
    break;
  case 'u':
    base = 10;
    fmt.sign_options = only_minus;
    break;
  case 'x':
  case 'X':
    base = 16;
    fmt.sign_options = only_minus;
    break;
  }

  do {
    n = unsigned_num % base;
    unsigned_num /= base;

    if (n < 10) {
      n += '0';
    } else {
      n -= 10;
      if (fmt.conversion_char == 'x') {
        n += 'a';
      } else {
        n += 'A';
      }
    }

    *--p = n;
    ++digits;
  } while (unsigned_num != 0);

  if (base == 8 && fmt.alternate_form && *p != '0') {
    *--p = '0';
    ++digits;
  }

  if (fmt.justification_options == zero_fill) {
    fmt.precision = fmt.field_width;

    if (minus || fmt.sign_options != only_minus) {
      --fmt.precision;
    }

    if (base == 16 && fmt.alternate_form) {
      fmt.precision -= 2;
    }
  }

  precision = fmt.precision;

  if (pBuf - p + precision > 509) {
    return 0;
  }

  while (digits < precision) {
    *--p = '0';
    ++digits;
  }

  if (base == 16 && fmt.alternate_form) {
    *--p = fmt.conversion_char;
    *--p = '0';
  }

  if (minus) {
    *--p = '-';
  } else if (fmt.sign_options == sign_always) {
    *--p = '+';
  } else if (fmt.sign_options == space_holder) {
    *--p = ' ';
  }

  return p;
}

static void round_wdecimal(decimal* dec, int new_length) {
  char c;
  char* p;
  int carry;

  if (new_length < 0) {
  return_zero:
    dec->exp = 0;
    dec->sig.length = 1;
    *dec->sig.text = '0';
    return;
  }

  if (new_length >= dec->sig.length) {
    return;
  }

  p = (char*)dec->sig.text + new_length + 1;
  c = *--p - '0';

  if (c == 5) {
    char* q = &((char*)dec->sig.text)[dec->sig.length];

    while (--q > p && *q == '0')
      ;
    carry = (q == p) ? p[-1] & 1 : 1;
  } else {
    carry = (c > 5);
  }

  while (new_length != 0) {
    c = *--p - '0' + carry;

    if ((carry = (c > 9)) != 0 || c == 0) {
      --new_length;
    } else {
      *p = c + '0';
      break;
    }
  }

  if (carry != 0) {
    dec->exp += 1;
    dec->sig.length = 1;
    *dec->sig.text = '1';
    return;
  } else if (new_length == 0) {
    goto return_zero;
  }

  dec->sig.length = new_length;
}

static wchar_t* float2wstr(long double num, wchar_t* buff, print_format format) {
  decimal dec;
  decform form;
  char out_buff[512];
  wchar_t* p;
  char* p2;
  char* q;
  int n, digits, sign;
  int int_digits, frac_digits;
  int radix_marker;

  radix_marker = '.';

  if (format.precision > 509) {
    return 0;
  }

  form.style = 0;
  form.digits = 0x20;
  __num2dec(&form, num, &dec);
  p2 = (char*)dec.sig.text + dec.sig.length;

  while (dec.sig.length > 1 && *--p2 == '0') {
    --dec.sig.length;
    ++dec.exp;
  }

  switch (*dec.sig.text) {
  case '0':
    dec.exp = 0;
    break;
  case 'I':
    if (num < 0) {
      p = buff - 5;

      if (iswupper(format.conversion_char)) {
        wcscpy(p, L"-INF");
      } else {
        wcscpy(p, L"-inf");
      }
    } else {
      p = buff - 4;
      if (iswupper(format.conversion_char)) {
        wcscpy(p, L"INF");
      } else {
        wcscpy(p, L"inf");
      }
    }

    return p;

  case 'N':
    p = buff - 4;
    wcscpy(p, L"NaN");
    return p;
  }

  dec.exp += dec.sig.length - 1;
  p2 = out_buff + sizeof(out_buff);
  *--p2 = 0;

  switch (format.conversion_char) {
  case 'g':
  case 'G':

    if (dec.sig.length > format.precision) {
      round_wdecimal(&dec, format.precision);
    }

    if (dec.exp < -4 || dec.exp >= format.precision) {
      if (format.alternate_form) {
        --format.precision;
      } else {
        format.precision = dec.sig.length - 1;
      }

      if (format.conversion_char == 'g') {
        format.conversion_char = 'e';
      } else {
        format.conversion_char = 'E';
      }

      goto e_format;
    }

    if (format.alternate_form) {
      format.precision -= dec.exp + 1;
    } else {
      if ((format.precision = dec.sig.length - (dec.exp + 1)) < 0) {
        format.precision = 0;
      }
    }

    goto f_format;

  case 'e':
  case 'E':
  e_format:

    if (dec.sig.length > format.precision + 1) {
      round_wdecimal(&dec, format.precision + 1);
    }

    n = dec.exp;
    sign = '+';

    if (n < 0) {
      n = -n;
      sign = '-';
    }

    for (digits = 0; n || digits < 2; ++digits) {
      *--p2 = n % 10 + '0';
      n /= 10;
    }

    *--p2 = sign;
    *--p2 = format.conversion_char;

    if (out_buff - p2 + format.precision > 509) {
      return 0;
    }

    if (dec.sig.length < format.precision + 1) {
      for (n = format.precision + 1 - dec.sig.length + 1; --n;) {
        *--p2 = '0';
      }
    }

    for (n = dec.sig.length, q = (char*)dec.sig.text + dec.sig.length; --n;) {
      *--p2 = *--q;
    }

    if (format.precision || format.alternate_form) {
      *--p2 = radix_marker;
    }

    *--p2 = *dec.sig.text;

    if (dec.sgn)
      *--p2 = '-';
    else if (format.sign_options == sign_always)
      *--p2 = '+';
    else if (format.sign_options == space_holder)
      *--p2 = ' ';

    break;

  case 'f':
  case 'F':
  f_format:

    if ((frac_digits = -dec.exp + dec.sig.length - 1) < 0)
      frac_digits = 0;

    if (frac_digits > format.precision) {
      round_wdecimal(&dec, dec.sig.length - (frac_digits - format.precision));

      if ((frac_digits = -dec.exp + dec.sig.length - 1) < 0)
        frac_digits = 0;
    }

    if ((int_digits = dec.exp + 1) < 0)
      int_digits = 0;

    if (int_digits + frac_digits > 509)
      return 0;

    q = (char*)dec.sig.text + dec.sig.length;

    for (n = 0; n < format.precision - frac_digits; ++n)
      *--p2 = '0';

    for (digits = 0; digits < frac_digits && digits < dec.sig.length; ++digits)
      *--p2 = *--q;

    for (; digits < frac_digits; ++digits)
      *--p2 = '0';

    if (format.precision || format.alternate_form)
      *--p2 = radix_marker;

    if (int_digits) {
      for (digits = 0; digits < int_digits - dec.sig.length; ++digits) {
        *--p2 = '0';
      }

      for (; digits < int_digits; ++digits) {
        *--p2 = *--q;
      }
    } else {
      *--p2 = '0';
    }

    if (dec.sgn) {
      *--p2 = '-';
    } else if (format.sign_options == sign_always) {
      *--p2 = '+';
    } else if (format.sign_options == space_holder) {
      *--p2 = ' ';
    }

    break;
  }

  p = buff - strlen(p2) - 1;
  mbstowcs(p, p2, strlen(p2));
  return p;
}

static int __wpformatter(void* (*WriteProc)(void*, const wchar_t*, size_t), void* WriteProcArg,
                        const wchar_t* format_str, va_list arg) {
  int num_chars, chars_written, field_width;
  const wchar_t* format_ptr;
  const wchar_t* curr_format;
  print_format format;
  s32 long_num;
  s64 long_long_num;
  long double long_double_num;
  wchar_t buff[512];
  wchar_t* buff_ptr;
  const wchar_t* string_end;
  wchar_t fill_char = ' ';

  format_ptr = format_str;
  chars_written = 0;

  while (*format_ptr) {
    if (!(curr_format = wcschr(format_ptr, '%'))) {
      num_chars = wcslen(format_ptr);
      chars_written += num_chars;

      if (num_chars && !(*WriteProc)(WriteProcArg, format_ptr, num_chars)) {
        return -1;
      }

      break;
    }

    num_chars = curr_format - format_ptr;
    chars_written += num_chars;

    if (num_chars && !(*WriteProc)(WriteProcArg, format_ptr, num_chars)) {
      return -1;
    }

    format_ptr = curr_format;
    // Preserve this runtime's argument-list indirection.
    format_ptr = parse_wformat(format_ptr, (va_list*)&arg, &format);

    switch (format.conversion_char) {
    case 'd':
    case 'i':
      if (format.argument_options == long_argument) {
        long_num = va_arg(arg, s32);
      } else if (format.argument_options == long_long_argument) {
        long_long_num = va_arg(arg, s64);
      } else {
        long_num = va_arg(arg, int);
      }

      if (format.argument_options == short_argument) {
        long_num = (s16)long_num;
      }


      if ((format.argument_options == long_long_argument)) {
        if (!(buff_ptr = longlong2wstr(long_long_num, buff + 512, format))) {
          goto conversion_error;
        }
      } else {
        if (!(buff_ptr = long2wstr(long_num, buff + 512, format))) {
          goto conversion_error;
        }
      }

      num_chars = buff + 512 - 1 - buff_ptr;
      break;

    case 'o':
    case 'u':
    case 'x':
    case 'X':
      if (format.argument_options == long_argument) {
        long_num = va_arg(arg, u32);
      } else if (format.argument_options == long_long_argument) {
        long_long_num = va_arg(arg, s64);
      } else {
        long_num = va_arg(arg, unsigned int);
      }

      if (format.argument_options == short_argument) {
        long_num = (u16)long_num;
      }


      if ((format.argument_options == long_long_argument)) {
        if (!(buff_ptr = longlong2wstr(long_long_num, buff + 512, format))) {
          goto conversion_error;
        }
      } else {
        if (!(buff_ptr = long2wstr(long_num, buff + 512, format))) {
          goto conversion_error;
        }
      }

      num_chars = buff + 512 - 1 - buff_ptr;
      break;

    case 'f':
    case 'F':
    case 'e':
    case 'E':
    case 'g':
    case 'G':
      if (format.argument_options == long_double_argument) {
        long_double_num = va_arg(arg, long double);
      } else {
        long_double_num = va_arg(arg, double);
      }

      if (!(buff_ptr = float2wstr(long_double_num, buff + 512, format))) {
        goto conversion_error;
      }

      num_chars = buff + 512 - 1 - buff_ptr;
      break;

    case 's':
      if (format.argument_options == wchar_argument) {
        wchar_t* wcs_ptr = va_arg(arg, wchar_t*);

        if (wcs_ptr == NULL) {
          wcs_ptr = L"";
        }

        if (format.alternate_form) {
          num_chars = (u8)*wcs_ptr++;
          if (format.precision_specified && num_chars > format.precision) {
            num_chars = format.precision;
          }
        } else if (format.precision_specified) {
          num_chars = format.precision;
          if ((string_end = wmemchr(wcs_ptr, 0, num_chars)) != 0) {
            num_chars = string_end - wcs_ptr;
          }
        } else {
          num_chars = wcslen(wcs_ptr);
        }
        buff_ptr = wcs_ptr;
      } else {
        const char* str_ptr = va_arg(arg, char*);

        if (str_ptr == NULL) {
          str_ptr = "";
        }

        if (format.alternate_form) {
          num_chars = (u8)*buff_ptr;
          if (format.precision_specified && num_chars > format.precision) {
            num_chars = format.precision;
          }
        } else if (format.precision_specified) {
          num_chars = format.precision;
          if ((string_end = memchr(str_ptr, 0, num_chars)) != 0) {
            num_chars = (const char*)string_end - str_ptr;
          }
        } else {
          num_chars = strlen(str_ptr);
        }

        if ((num_chars = mbstowcs(buff, str_ptr, num_chars)) < 0) {
          goto conversion_error;
        }
        buff_ptr = buff;
      }
      break;

    case 'n':
      buff_ptr = va_arg(arg, wchar_t*);

      switch (format.argument_options) {
      case normal_argument:
        *(int*)buff_ptr = chars_written;
        break;
      case short_argument:
        *(s16*)buff_ptr = chars_written;
        break;
      case long_argument:
        *(s32*)buff_ptr = chars_written;
        break;
      case long_long_argument:
        *(s64*)buff_ptr = chars_written;
        break;
      }

      continue;

    case 'c':
      buff_ptr = buff;
      if (format.argument_options == wchar_argument) {
        *buff_ptr = va_arg(arg, long);
        num_chars = 1;
      } else {
        char ch = va_arg(arg, int);
        num_chars = mbtowc(buff_ptr, &ch, 1);
      }
      break;

    case '%':
      buff_ptr = buff;
      *buff_ptr = '%';
      num_chars = 1;
      break;

    case 0xFFFF:
    default:
    conversion_error:
      num_chars = wcslen(curr_format);
      chars_written += num_chars;

      if (num_chars && !(*WriteProc)(WriteProcArg, curr_format, num_chars)) {
        return -1;
      }

      return chars_written;
      break;
    }

    field_width = num_chars;

    if (format.justification_options != left_justification) {
      fill_char = (format.justification_options == zero_fill) ? '0' : ' ';

      if (((*buff_ptr == '+') || (*buff_ptr == '-') || (*buff_ptr == ' ')) && (fill_char == '0')) {
        if ((*WriteProc)(WriteProcArg, buff_ptr, 1) == 0) {
          return -1;
        }

        ++buff_ptr;
        num_chars--;
      }

      while (field_width < format.field_width) {
        if ((*WriteProc)(WriteProcArg, &fill_char, 1) == 0) {
          return -1;
        }

        ++field_width;
      }
    }

    if (num_chars && !(*WriteProc)(WriteProcArg, buff_ptr, num_chars)) {
      return -1;
    }

    if (format.justification_options == left_justification) {
      while (field_width < format.field_width) {
        wchar_t blank = ' ';

        if ((*WriteProc)(WriteProcArg, &blank, 1) == 0) {
          return -1;
        }

        ++field_width;
      }
    }

    chars_written += field_width;
  }

  return chars_written;
}

static void* __wStringWrite(void* control, const wchar_t* data, size_t count) {
  __wOutStrCtrl* out = control;
  size_t written = out->written;
  size_t capacity = out->capacity;
  size_t to_write = written + count <= capacity ? count : capacity - written;
  void* result = wmemcpy(out->dest + written, data, to_write);
  out->written += to_write;
  return result;
}

int swprintf(wchar_t* dest, size_t count, const wchar_t* format, ...) {
  int written;
  va_list args;
  __wOutStrCtrl control;

  va_start(args, format);
  control.dest = dest;
  control.capacity = count;
  control.written = 0;
  written = __wpformatter(&__wStringWrite, &control, format, args);
  va_end(args);

  if (written >= 0) {
    if ((size_t)written < count) {
      dest[written] = 0;
    } else {
      dest[count - 1] = 0;
      return -1;
    }
  }
  return written;
}
