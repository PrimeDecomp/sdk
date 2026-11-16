#include <SDKVersion.h>
#include "string.h"
#ifdef RVL_PROTO_RUNTIME
#include <errno.h>
#include <stdio.h>
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
size_t strlen(const char* str) {
  size_t len = -1;
  unsigned char* p = (unsigned char*)str - 1;
  do {
    len++;
  } while (*++p);
  return len;
}
#elif SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
size_t strlen(const char* str) {
  size_t len = -1;
  unsigned char* p = (unsigned char*)str - 1;
  do {
    len++;
  } while (*++p);
  return len;
}

#endif

char*(strcpy)(char* dst, const char* src) {
  unsigned char *destb, *fromb;
  unsigned int w, t, align;
  unsigned int k1;
  unsigned int k2;

  fromb = (unsigned char*)src;
  destb = (unsigned char*)dst;

  if ((align = ((int)fromb & 3)) != ((int)destb & 3)) {
    goto bytecopy;
  }

  if (align) {
    if ((*destb = *fromb) == 0) {
      return dst;
    }

    for (align = 3 - align; align; align--) {
      if ((*(++destb) = *(++fromb)) == 0) {
        return dst;
      }
    }
    ++destb;
    ++fromb;
  }

  k1 = 0x80808080;
  k2 = 0xfefefeff;

  w = *((int*)(fromb));
  t = w + k2;
#if SDK_VERSION >= SDK_VERSION_RVL_2007_06
  t &= ~w;
#endif
  t &= k1;
  if (t) {
    goto bytecopy;
  }

  --((int*)(destb));

  do {
    *(++((int*)(destb))) = w;
    w = *(++((int*)(fromb)));

    t = w + k2;
#if SDK_VERSION >= SDK_VERSION_RVL_2007_06
    t &= ~w;
#endif
    t &= k1;
    if (t)
      goto adjust;
  } while (1);

adjust:
  ++((int*)(destb));

bytecopy:
  if ((*destb = *fromb) == 0)
    return (dst);
  do {
    if ((*(++destb) = *(++fromb)) == 0)
      return dst;
  } while (1);

  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  const unsigned char* p = (const unsigned char*)src - 1;
  unsigned char* q = (unsigned char*)dst - 1;
  unsigned char zero = 0;

  n++;

  while (--n)
    if (!(*++q = *++p)) {
      while (--n)
        *++q = 0;
      break;
    }
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04

  return dst;
}

char* strcat(char* dst, const char* src) {
  const unsigned char* p = (unsigned char*)src - 1;
  unsigned char* q = (unsigned char*)dst - 1;

  while (*++q)
    ;

  q--;

  while (*++q = *++p)
    ;
#endif

  return dst;
}

int strcmp(const char* str1, const char* str2) {
  unsigned char* left = (unsigned char*)str1;
  unsigned char* right = (unsigned char*)str2;
  unsigned int k1, k2, align, l1, r1, x;

  l1 = *left;
  r1 = *right;
  if (l1 - r1) {
    return l1 - r1;
  }

  if ((align = ((int)left & 3)) != ((int)right & 3)) {
    goto bytecopy;
  }
  if (align) {
    if (l1 == 0) {
      return 0;
    }
    for (align = 3 - align; align; align--) {
      l1 = *(++left);
      r1 = *(++right);
      if (l1 - r1) {
        return l1 - r1;
      }
      if (l1 == 0) {
        return 0;
      }
    }
    left++;
    right++;
  }

  k1 = 0x80808080;
  k2 = 0xfefefeff;

  l1 = *(int*)left;
  r1 = *(int*)right;
  x = l1 + k2;
#if SDK_VERSION >= SDK_VERSION_RVL_2007_06
  x &= ~l1;
#endif
  if (x & k1) {
    goto adjust;
  }
  while (l1 == r1) {
    l1 = *(++((int*)(left)));
    r1 = *(++((int*)(right)));
    x = l1 + k2;
    if (x & k1) {
      goto adjust;
    }
  }

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
  if (l1 > r1) {
    return 1;
  }

  return -1;
#endif

adjust:
  l1 = *left;
  r1 = *right;
  if (l1 - r1) {
    return l1 - r1;
  }

bytecopy:
  if (l1 == 0) {
    return 0;
  }

  do {
    l1 = *(++left);
    r1 = *(++right);
    if (l1 - r1) {
      return l1 - r1;
    }
    if (l1 == 0) {
      return 0;
    }
  } while (1);
}

int strncmp(const char* str1, const char* str2, size_t n) {
  const unsigned char* p1 = (unsigned char*)str1 - 1;
  const unsigned char* p2 = (unsigned char*)str2 - 1;
  unsigned long c1, c2;

  n++;

  while (--n) {
    if ((c1 = *++p1) != (c2 = *++p2)) {
      return (c1 - c2);
    } else if (!c1) {
      break;
    }
  }

  return 0;
}

char* strchr(const char* str, int chr) {
  const unsigned char* p = (unsigned char*)str - 1;
  unsigned long c = (chr & 0xff);
  unsigned long ch;

  while (ch = *++p) {
    if (ch == c)
      return ((char*)p);
  }

  return (c ? 0 : (char*)p);
}

#ifdef RVL_PROTO_RUNTIME
char* strncat(char* dst, const char* src, size_t count) {
  const unsigned char* p = (const unsigned char*)src - 1;
  unsigned char* q = (unsigned char*)dst - 1;

  while (*++q)
    ;
  q--;
  count++;
  while (--count) {
    if (!(*++q = *++p)) {
      q--;
      break;
    }
  }
  *++q = 0;

  return dst;
}

int strcoll(const char* str1, const char* str2) {
  return strcmp(str1, str2);
}

typedef unsigned char char_map[32]; // Guessed name

#define set_char_map(map, ch) ((map)[(unsigned char)(ch) >> 3] |= (1 << ((ch) & 7)))
#define tst_char_map(map, ch) ((map)[(unsigned char)(ch) >> 3] & (1 << ((ch) & 7)))

char* strpbrk(const char* str, const char* set) {
  const unsigned char* p;
  int c;
  char_map map = {0};

  p = (const unsigned char*)set - 1;
  while ((c = *++p) != 0)
    set_char_map(map, c);

  p = (const unsigned char*)str - 1;
  while ((c = *++p) != 0) {
    if (tst_char_map(map, c))
      return (char*)p;
  }
  return NULL;
}

size_t strcspn(const char* str, const char* set) {
  const unsigned char* p;
  int c;
  char_map map = {0};

  p = (const unsigned char*)set - 1;
  while ((c = *++p) != 0)
    set_char_map(map, c);

  p = (const unsigned char*)str - 1;
  while ((c = *++p) != 0) {
    if (tst_char_map(map, c))
      break;
  }
  return p - (const unsigned char*)str;
}

char* strtok(char* str, const char* set) {
  static unsigned char* strtok_n = (unsigned char*)""; // Guessed name
  static unsigned char* strtok_s = (unsigned char*)""; // Guessed name
  unsigned char *p, *q;
  int c;
  char_map map = {0};

  if (str)
    strtok_s = (unsigned char*)str;

  p = (unsigned char*)set - 1;
  while ((c = *++p) != 0)
    set_char_map(map, c);

  p = strtok_s - 1;
  while ((c = *++p) != 0) {
    if (!tst_char_map(map, c))
      break;
  }
  if (!c) {
    strtok_s = strtok_n;
    return NULL;
  }

  q = p;
  while ((c = *++p) != 0) {
    if (tst_char_map(map, c))
      break;
  }
  if (!c) {
    strtok_s = strtok_n;
  } else {
    strtok_s = p + 1;
    *p = 0;
  }

  return (char*)q;
}

char* strstr(const char* str, const char* pattern) {
  unsigned char* s1 = (unsigned char*)str - 1;
  unsigned char* p1 = (unsigned char*)pattern - 1;
  unsigned long first, c1, c2;

  if (pattern == NULL || !(first = *++p1))
    return (char*)str;

  while ((c1 = *++s1) != 0) {
    if (c1 == first) {
      const unsigned char* s2 = s1 - 1;
      const unsigned char* p2 = p1 - 1;

      while ((c1 = *++s2) == (c2 = *++p2) && c1)
        ;
      if (!c2)
        return (char*)s1;
    }
  }
  return NULL;
}

char* __strerror(int error, char* str) { // Guessed name
  switch (error) {
  case E2BIG:
    strcpy(str, "Argument list too long");
    break;
  case EACCES:
    strcpy(str, "Permission denied");
    break;
  case EAGAIN:
    strcpy(str, "Resource temporarily unavailable");
    break;
  case EBADF:
    strcpy(str, "Bad file descriptor");
    break;
  case EBUSY:
    strcpy(str, "Device busy");
    break;
  case ECHILD:
    strcpy(str, "No child processes");
    break;
  case EDEADLK:
    strcpy(str, "Resource deadlock avoided");
    break;
  case EDOM:
    strcpy(str, "Numerical argument out of domain");
    break;
  case EEXIST:
    strcpy(str, "File exists");
    break;
  case EFAULT:
    strcpy(str, "Bad address");
    break;
  case EFBIG:
    strcpy(str, "File too large");
    break;
  case EFPOS:
    strcpy(str, "File Position Error");
    break;
  case EILSEQ:
    strcpy(str, "Wide character encoding error");
    break;
  case EINTR:
    strcpy(str, "Interrupted system call");
    break;
  case EINVAL:
    strcpy(str, "Invalid argument");
    break;
  case EIO:
    strcpy(str, "Input/output error");
    break;
  case EISDIR:
    strcpy(str, "Is a directory");
    break;
  case EMFILE:
    strcpy(str, "Too many open files");
    break;
  case EMLINK:
    strcpy(str, "Too many links");
    break;
  case ENAMETOOLONG:
    strcpy(str, "File name too long");
    break;
  case ENFILE:
    strcpy(str, "Too many open files in system");
    break;
  case ENODEV:
    strcpy(str, "Operation not supported by device");
    break;
  case ENOENT:
    strcpy(str, "No such file or directory");
    break;
  case ENOERR:
    strcpy(str, "No error detected");
    break;
  case ENOEXEC:
    strcpy(str, "Exec format error");
    break;
  case ENOLCK:
    strcpy(str, "No locks available");
    break;
  case ENOMEM:
    strcpy(str, "Cannot allocate memory");
    break;
  case ENOSPC:
    strcpy(str, "No space left on device");
    break;
  case ENOSYS:
    strcpy(str, "Function not implemented");
    break;
  case ENOTDIR:
    strcpy(str, "Not a directory");
    break;
  case ENOTEMPTY:
    strcpy(str, "Directory not empty");
    break;
  case ENOTTY:
    strcpy(str, "Inappropriate ioctl for device");
    break;
  case ENXIO:
    strcpy(str, "Device not configured");
    break;
  case EPERM:
    strcpy(str, "Operation not permitted");
    break;
  case EPIPE:
    strcpy(str, "Broken pipe");
    break;
  case ERANGE:
    strcpy(str, "Result too large");
    break;
  case EROFS:
    strcpy(str, "Read-only file system");
    break;
  case ESIGPARM:
    strcpy(str, "Signal error");
    break;
  case ESPIPE:
    strcpy(str, "Illegal seek");
    break;
  case ESRCH:
    strcpy(str, "No such process");
    break;
  case EUNKNOWN:
    strcpy(str, "Unknown error");
    break;
  case EXDEV:
    strcpy(str, "Cross-device link");
    break;
  default:
    sprintf(str, "Unknown Error (%d)", error);
    break;
  }

  return str;
}

char* strerror(int error) {
  static char error_string[40];

  return __strerror(error, error_string);
}
#endif
