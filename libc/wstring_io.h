#ifndef _WSTRING_IO_H_
#define _WSTRING_IO_H_

#include <wchar.h>

#ifndef WEOF
#define WEOF ((wint_t)-1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Guessed name
typedef struct {
  const wchar_t* NextChar;
  int NullCharDetected;
} __wInStrCtrl;

// Guessed name
wint_t __wStringRead(void* input, wint_t ch, int action);

#ifdef __cplusplus
}
#endif

#endif
