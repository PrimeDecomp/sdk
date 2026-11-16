#include <stdio.h>
#include <wstring_io.h>

wint_t __wStringRead(void* input, wint_t ch, int action) {
  wint_t next;
  __wInStrCtrl* ctrl = (__wInStrCtrl*)input;
  switch (action) {
  case __GetChar:
    next = *ctrl->NextChar;
    if (next == 0) {
      ctrl->NullCharDetected = 1;
      return WEOF;
    } else {
      ++ctrl->NextChar;
      return next;
    }
  case __UngetChar:
    if (!ctrl->NullCharDetected) {
      --ctrl->NextChar;
    } else {
      ctrl->NullCharDetected = 0;
    }
    return ch;
  case __CheckForError:
    return ctrl->NullCharDetected;
  }
  return 0;
}
