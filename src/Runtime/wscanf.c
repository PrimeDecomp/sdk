#include <stdio.h>
#include <wstring_io.h>

wint_t __wStringRead(void* input, wint_t ch, int action) {
  __wInStrCtrl* ctrl = (__wInStrCtrl*)input;
  switch (action) {
  case __GetChar: {
    const wchar_t next = *ctrl->NextChar;
    if (next == 0) {
      ctrl->NullCharDetected = 1;
      return WEOF;
    }
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
