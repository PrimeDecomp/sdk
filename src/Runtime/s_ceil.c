/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
#include "fdlibm.h"

static const double huge = 1.0e300;

double ceil(double x) {
  int high = __HI(x);
  unsigned low = __LO(x);
  int exponent = ((high >> 20) & 0x7ff) - 0x3ff;
  unsigned mask, next;

  if (exponent < 20) {
    if (exponent < 0) {
      /* Raise inexact for a nonzero fraction. */
      if (huge + x > 0.0) {
        if (high < 0) {
          high = 0x80000000;
          low = 0;
        } else if ((high | low) != 0) {
          high = 0x3ff00000;
          low = 0;
        }
      }
    } else {
      mask = 0x000fffff >> exponent;
      if (((high & mask) | low) == 0) {
        return x;
      }
      if (huge + x > 0.0) {
        if (high > 0) {
          high += 0x00100000 >> exponent;
        }
        high &= ~mask;
        low = 0;
      }
    }
  } else if (exponent > 51) {
    if (exponent == 0x400) {
      return x + x;
    }
    return x;
  } else {
    mask = 0xffffffffU >> (exponent - 20);
    if ((low & mask) == 0) {
      return x;
    }
    if (huge + x > 0.0) {
      if (high > 0) {
        if (exponent == 20) {
          ++high;
        } else {
          next = low + (1U << (52 - exponent));
          if (next < low) {
            ++high;
          }
          low = next;
        }
      }
      low &= ~mask;
    }
  }
  __HI(x) = high;
  __LO(x) = low;
  return x;
}
