#include <ansi_files.h>
#include <buffer_io.h>
#include <critical_regions.h>
#include <stdio.h>

int __put_char(int c, FILE* file); // Guessed name
void __stdio_atexit(void);

#define __putc(c, file)                                                                            \
  (fwide((file), -1) >= 0 ? EOF                                                                     \
                         : (file)->buffer_len--                                                    \
                               ? (int)(*(file)->buffer_ptr++ = (unsigned char)(c))                  \
                               : __put_char((c), (file)))

int __getc(FILE* file) {
  int state, result;

  file->buffer_len = 0;
  if (file->state.error || file->mode.file_kind == __closed_file)
    return EOF;

  state = file->state.io_state;
  if (state == __writing || !(file->mode.io_mode & __read)) {
    set_error(file);
    return EOF;
  }

  if (state >= __rereading) {
    file->state.io_state--;
    if (state == __rereading)
      file->buffer_len = file->saved_buffer_len;
    return file->ungetc_buffer[state - __rereading];
  } else {
    file->state.io_state = __reading;
  }

  result = __load_buffer(file, NULL, __align_buffer);
  if (result || !file->buffer_len) {
    if (result == __io_error)
      set_error(file);
    else
      set_eof(file);
    return EOF;
  }

  file->buffer_len--;
  return *file->buffer_ptr++;
}

char* fgets(char* s, int count, FILE* file) {
  char* p = s;
  int c;

  if (--count < 0)
    return NULL;

  __begin_critical_region(files_access);
  if (count) {
    do {
      c = getc(file);
      if (c == EOF) {
        if (file->state.eof && p != s)
          break;
        else {
          __end_critical_region(files_access);
          return NULL;
        }
      }
      *p++ = c;
    } while (c != '\n' && --count);
  }
  __end_critical_region(files_access);

  *p = 0;
  return s;
}

int ungetc(int c, FILE* file) {
  int state = file->state.io_state;

  if (fwide(file, -1) >= 0)
    return EOF;
  if (state == __writing || state == __rereading + __ungetc_buffer_size - 1 || c == EOF)
    return EOF;

  __begin_critical_region(files_access);
  if (state < __rereading) {
    file->saved_buffer_len = file->buffer_len;
    file->buffer_len = 0;
    state = file->state.io_state = __rereading;
  } else {
    state = ++file->state.io_state;
  }
  file->ungetc_buffer[state - __rereading] = c;
  file->state.eof = 0;
  __end_critical_region(files_access);

  return (unsigned char)c;
}

int __put_char(int c, FILE* file) {
  int kind = file->mode.file_kind;

  file->buffer_len = 0;
  if (file->state.error || kind == __closed_file)
    return EOF;

  if (kind == __console_file)
    __stdio_atexit();

  if (file->state.io_state == __neutral) {
    if (file->mode.io_mode & __write) {
      if (file->mode.io_mode & __append) {
        if (fseek(file, 0, SEEK_END))
          return 0;
      }
      file->state.io_state = __writing;
      __prep_buffer(file);
    }
  }

  if (file->state.io_state != __writing) {
    set_error(file);
    return EOF;
  }

  if (file->mode.buffer_mode == _IOFBF || file->buffer_ptr - file->buffer == file->buffer_size) {
    if (__flush_buffer(file, NULL)) {
      set_error(file);
      return EOF;
    }
  }

  file->buffer_len--;
  *file->buffer_ptr++ = c;

  if (file->mode.buffer_mode != _IOFBF) {
    if (file->mode.buffer_mode == _IONBF || c == '\n') {
      if (__flush_buffer(file, NULL)) {
        set_error(file);
        return EOF;
      }
    }
    file->buffer_len = 0;
  }

  return (unsigned char)c;
}

int fputs(const char* s, FILE* file) {
  int c;
  int result = 0;

  __begin_critical_region(files_access);
  while ((c = *s++) != 0) {
    if (__putc(c, file) == EOF) {
      result = EOF;
      break;
    }
  }
  __end_critical_region(files_access);

  return result;
}

int puts(const char* s) {
  int c;
  int result = 0;
  FILE* file = stdout;

  __begin_critical_region(files_access);
  while ((c = *s++) != 0) {
    if (__putc(c, file) == EOF) {
      result = EOF;
      break;
    }
  }
  if (result != EOF)
    result = __putc('\n', file);
  __end_critical_region(files_access);

  return result;
}
