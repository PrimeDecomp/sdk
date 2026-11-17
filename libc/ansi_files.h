#ifndef _DOLPHIN_ANSI_FILES_H
#define _DOLPHIN_ANSI_FILES_H
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

#define set_eof(file)                                                                              \
  do {                                                                                             \
    (file)->state.io_state = __neutral;                                                            \
    (file)->state.eof = 1;                                                                         \
    (file)->buffer_len = 0;                                                                        \
  } while (0)

#define set_error(file)                                                                            \
  do {                                                                                             \
    (file)->state.error = 1;                                                                       \
    (file)->buffer_len = 0;                                                                        \
  } while (0)

int __flush_buffer(FILE* file, size_t* length);
void __prep_buffer(FILE* file);
int __flush_all();
int __flush_line_buffered_output_files(void); // Guessed name

extern FILE __files[4];

#ifndef stdin
#define stdin &(__files[0])
#define stdout &(__files[1])
#define stderr &(__files[2])
#endif

#ifdef __cplusplus
};
#endif // ifdef __cplusplus

#endif
