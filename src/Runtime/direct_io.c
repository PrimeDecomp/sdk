#include <SDKVersion.h>
#include <ansi_files.h>
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
#include <critical_regions.h>
#endif
#include <stdio.h>
#include <buffer_io.h>

size_t fread(void* ptr, size_t memb_size, size_t num_memb, FILE* file) {
  size_t retval;

#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
  __begin_critical_region(files_access);
#endif
  retval = __fread(ptr, memb_size, num_memb, file);
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
  __end_critical_region(files_access);
#endif

  return retval;
}

size_t __fread(void* ptr, size_t memb_size, size_t num_memb, FILE* file) { // Guessed name
  unsigned char* read_ptr;
  size_t num_bytes, bytes_to_go, bytes_read;
  int ioresult, always_buffer;

#ifndef __NO_WIDE_CHAR
  if (fwide(file, 0) == 0)
    fwide(file, -1);
#endif

  bytes_to_go = memb_size * num_memb;
  if (!bytes_to_go || file->state.error || file->mode.file_kind == __closed_file)
    return 0;

  always_buffer = !file->mode.binary_io || file->mode.buffer_mode == _IOFBF;

  if (file->state.io_state == __neutral) {
    if (file->mode.io_mode & __read) {
      file->state.io_state = __reading;
      file->buffer_len = 0;
    }
  }

  if (file->state.io_state < __reading) {
    set_error(file);
    return 0;
  }

  if (file->mode.buffer_mode & (_IOLBF | _IONBF)) {
    if (__flush_line_buffered_output_files()) {
      set_error(file);
      return 0;
    }
  }

  read_ptr = (unsigned char*)ptr;
  bytes_read = 0;

  if (bytes_to_go && file->state.io_state >= __rereading) {
    do {
#ifndef __NO_WIDE_CHAR
      if (fwide(file, 0) == 1) {
        *(wchar_t*)read_ptr = file->ungetwc_buffer[file->state.io_state - __rereading];
        read_ptr += sizeof(wchar_t);
        bytes_read += sizeof(wchar_t);
        bytes_to_go -= sizeof(wchar_t);
      } else {
#endif
        *read_ptr++ = file->ungetc_buffer[file->state.io_state - __rereading];
        bytes_read++;
        bytes_to_go--;
#ifndef __NO_WIDE_CHAR
      }
#endif
      file->state.io_state--;
    } while (bytes_to_go && file->state.io_state >= __rereading);

    if (file->state.io_state == __reading)
      file->buffer_len = file->saved_buffer_len;
  }

  if (bytes_to_go && (file->buffer_len || always_buffer)) {
    do {
      if (!file->buffer_len) {
        ioresult = __load_buffer(file, NULL, __align_buffer);
        if (ioresult) {
          if (ioresult == __io_error)
            set_error(file);
          else
            set_eof(file);
          bytes_to_go = 0;
          break;
        }
      }

      num_bytes = file->buffer_len;
      if (num_bytes > bytes_to_go)
        num_bytes = bytes_to_go;

      memcpy(read_ptr, file->buffer_ptr, num_bytes);
      read_ptr += num_bytes;
      bytes_read += num_bytes;
      bytes_to_go -= num_bytes;

      file->buffer_ptr += num_bytes;
      file->buffer_len -= num_bytes;
    } while (bytes_to_go && always_buffer);
  }

  if (bytes_to_go && !always_buffer) {
    unsigned char* save_buffer = file->buffer;
    size_t save_size = file->buffer_size;

    file->buffer = read_ptr;
    file->buffer_size = bytes_to_go;

    ioresult = __load_buffer(file, &num_bytes, __dont_align_buffer);
    if (ioresult) {
      if (ioresult == __io_error)
        set_error(file);
      else
        set_eof(file);
    }

    bytes_read += num_bytes;
    file->buffer = save_buffer;
    file->buffer_size = save_size;

    __prep_buffer(file);
    file->buffer_len = 0;
  }

  return bytes_read / memb_size;
}

size_t fwrite(const void* ptr, size_t memb_size, size_t num_memb, FILE* file) {
  size_t retval;
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
  __begin_critical_region(files_access);
#endif
  retval = __fwrite(ptr, memb_size, num_memb, file);
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
  __end_critical_region(files_access);
#endif
  return (retval);
}

size_t __fwrite(const void* ptr, size_t memb_size, size_t num_memb, FILE* file) {
  unsigned char* write_ptr;
  size_t num_bytes, bytes_to_go, bytes_written;
  int ioresult, always_buffer;

#ifndef __NO_WIDE_CHAR
  if (fwide(file, 0) == 0)
    fwide(file, -1);
#endif

  bytes_to_go = memb_size * num_memb;

  if (!bytes_to_go || file->state.error || file->mode.file_kind == __closed_file)
    return 0;

  if (file->mode.file_kind == __console_file)
    __stdio_atexit();

  always_buffer =
      !file->mode.binary_io || file->mode.buffer_mode == _IOFBF || file->mode.buffer_mode == _IOLBF;

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
    return 0;
  }

  write_ptr = (unsigned char*)ptr;
  bytes_written = 0;

  if (bytes_to_go && (file->buffer_ptr != file->buffer || always_buffer)) {
    file->buffer_len = file->buffer_size - (file->buffer_ptr - file->buffer);

    do {
      unsigned char* newline = NULL;

      num_bytes = file->buffer_len;

      if (num_bytes > bytes_to_go)
        num_bytes = bytes_to_go;
      if (file->mode.buffer_mode == _IOLBF && num_bytes)
        if ((newline = (unsigned char*)__memrchr(write_ptr, '\n', num_bytes)) != NULL)
          num_bytes = newline + 1 - write_ptr;

      if (num_bytes) {
        memcpy(file->buffer_ptr, write_ptr, num_bytes);

        write_ptr += num_bytes;
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
        bytes_written += num_bytes;
#endif
        bytes_to_go -= num_bytes;

        file->buffer_ptr += num_bytes;
        file->buffer_len -= num_bytes;
      }
      if (!file->buffer_len || newline != NULL || (file->mode.buffer_mode == _IONBF)) {
        ioresult = __flush_buffer(file, NULL);

        if (ioresult) {
          set_error(file);
          bytes_to_go = 0;
          break;
        }
      }
#if SDK_VERSION >= SDK_VERSION_RVL_2007_06
      bytes_written += num_bytes;
#endif
    } while (bytes_to_go && always_buffer);
  }

  if (bytes_to_go && !always_buffer) {
    unsigned char* save_buffer = file->buffer;
    size_t save_size = file->buffer_size;

    file->buffer = write_ptr;
    file->buffer_size = bytes_to_go;
    file->buffer_ptr = write_ptr + bytes_to_go;

    if (__flush_buffer(file, &num_bytes) != __no_io_error)
      set_error(file);

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
    bytes_written += num_bytes;
#elif SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
      bytes_written += num_bytes;
#else
    else
      bytes_written += num_bytes;
#endif

    file->buffer = save_buffer;
    file->buffer_size = save_size;

    __prep_buffer(file);

    file->buffer_len = 0;
  }

  if (file->mode.buffer_mode != _IOFBF)
    file->buffer_len = 0;

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2004_04
  return ((bytes_written + memb_size - 1) / memb_size);
#else
  return bytes_written / memb_size;
#endif
}
