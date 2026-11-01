#ifndef _LOCALE_H_
#define _LOCALE_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lconv {
  char* decimal_point;
  char* thousands_sep;
  char* grouping;
  char* mon_decimal_point;
  char* mon_thousands_sep;
  char* mon_grouping;
  char* positive_sign;
  char* negative_sign;
  char* currency_symbol;
  char frac_digits;
  char p_cs_precedes;
  char n_cs_precedes;
  char p_sep_by_space;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
  char* int_curr_symbol;
  char int_frac_digits;
  char int_p_cs_precedes;
  char int_n_cs_precedes;
  char int_p_sep_by_space;
  char int_n_sep_by_space;
  char int_p_sign_posn;
  char int_n_sign_posn;
};

typedef int (*__decode_mbyte)(unsigned short*, const char*, size_t);
typedef int (*__encode_mbyte)(char*, unsigned short);

// Guessed name
struct _loc_mon_cmpt {
  char CmptName[8];
  char* mon_decimal_point;
  char* mon_thousands_sep;
  char* mon_grouping;
  char* positive_sign;
  char* negative_sign;
  char* currency_symbol;
  char frac_digits;
  char p_cs_precedes;
  char n_cs_precedes;
  char p_sep_by_space;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
  char* int_curr_symbol;
  char int_frac_digits;
  char int_p_cs_precedes;
  char int_n_cs_precedes;
  char int_p_sep_by_space;
  char int_n_sep_by_space;
  char int_p_sign_posn;
  char int_n_sign_posn;
};

// Guessed name
struct _loc_num_cmpt {
  char CmptName[8];
  char* decimal_point;
  char* thousands_sep;
  char* grouping;
};

// Guessed name
struct _loc_time_cmpt {
  char CmptName[8];
  const char* am_pm;
  const char* DateTime_Format;
  const char* Twelve_hr_format;
  const char* Date_Format;
  const char* Time_Format;
  const char* Day_Names;
  const char* MonthNames;
  char* TimeZone;
};

// Guessed name
struct _loc_coll_cmpt {
  char name[8];
  int char_start;
  int char_coll_tab_size;
  short char_spec;
  unsigned short* char_coll_table_ptr;
  unsigned short* wchar_coll_seq_ptr;
};

// Guessed name
struct _loc_ctype_cmpt {
  char name[8];
  const unsigned short* ctype_map_ptr;
  const unsigned char* upper_map_ptr;
  const unsigned char* lower_map_ptr;
  const unsigned short* wctype_map_ptr;
  const unsigned short* wupper_map_ptr;
  const unsigned short* wlower_map_ptr;
  __decode_mbyte decode_mb;
  __encode_mbyte encode_wc;
};

// Guessed name
struct __locale {
  struct __locale* next_locale;
  char name[0x30];
  struct _loc_coll_cmpt* coll_cmpt_ptr;
  struct _loc_ctype_cmpt* ctype_cmpt_ptr;
  struct _loc_mon_cmpt* mon_cmpt_ptr;
  struct _loc_num_cmpt* num_cmpt_ptr;
  struct _loc_time_cmpt* time_cmpt_ptr;
};

// Guessed name
extern struct __locale _current_locale;

// Guessed names
extern const unsigned short __wctype_mapC[256];
extern const unsigned short __wupper_mapC[256];
extern const unsigned short __wlower_mapC[256];
// Guessed name
int __mbtowc_noconv(unsigned short* dst, const char* src, size_t n);
// Guessed name
int __wctomb_noconv(char* dst, unsigned short ch);

extern struct lconv __lconv;

#ifdef __cplusplus
}
#endif

#endif
