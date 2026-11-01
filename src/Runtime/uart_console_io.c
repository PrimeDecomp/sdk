#include <SDKVersion.h>
#include "console_io.h"
#include <dolphin/os.h>

int __TRK_write_console(__file_handle handle, unsigned char* buffer, size_t* count,
                        __idle_proc idle_proc);

static inline int __init_uart_console(void) {
  int err = 0;
  static int initialized = 0;

  if (initialized == 0) {
    err = InitializeUART(57600);

    if (err == 0)
      initialized = 1;
  }

  return (err);
}
int __write_console(__file_handle handle, unsigned char* buffer, size_t* count,
                    __idle_proc idle_proc) {
#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  if (__init_uart_console() != 0) {
    return 1;
  }
#else
  if ((OSGetConsoleType() & OS_CONSOLE_TDEV) == 0) {
    if (__init_uart_console() != 0) {
      return 1;
    }
#endif

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
  if (WriteUARTN(buffer, *count) != 0) {
    *count = 0;
    return 1;
#else
    if (WriteUARTN(buffer, *count) != 0) {
      *count = 0;
      return 1;
    }
#endif
  }

  __TRK_write_console(handle, buffer, count, idle_proc);
  return 0;
}

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
int __close_console() { return 0; }
#else
int __close_console(__file_handle handle) { return 0; }
#endif
