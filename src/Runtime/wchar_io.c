#include <stdio.h>

#ifdef RVL_PROTO_RUNTIME
#include <critical_regions.h>
#include <wstring_io.h>

int fputws(const wchar_t* str, FILE* stream) {
  const wchar_t* cursor = str;
  int result = 1;

  if (fwide(stream, 1) <= 0)
    return WEOF;

  __begin_critical_region(files_access);
  while (*cursor != L'\0') {
    wchar_t ch = *cursor;
    if (__fwrite(&ch, sizeof(ch), 1, stream) == 0)
      result = WEOF;
    ++cursor;
  }
  __end_critical_region(files_access);

  return result;
}
#endif

int fwide(FILE* stream, int mode) {
  int orientation;
  int result;

  if ((stream == NULL) || (stream->mode.file_kind == __closed_file))
    return 0;
  orientation = stream->mode.file_orientation;
  switch (orientation) {
  case __unoriented:
    if (mode > 0)
      stream->mode.file_orientation = __wide_oriented;
    else if (mode < 0)
      stream->mode.file_orientation = __char_oriented;
    result = mode;
    break;

  case __wide_oriented:
    result = 1;
    break;

  case __char_oriented:
    result = -1;
    break;
  }
  return result;
}
