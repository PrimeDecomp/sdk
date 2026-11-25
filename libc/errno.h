#ifndef _ERRNO_H_
#define _ERRNO_H_

#include <msl_c_version.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int errno;

#define ENOERR 0
#define EDOM 33
#define ERANGE 34
#define EFPOS 40

#if __MSL__ == MSL_C_VERSION_8_0
#define E2BIG 7
#define EACCES 13
#define EAGAIN 11
#define EBADF 9
#define EBUSY 16
#define ECHILD 10
#define EDEADLK 35
#define EEXIST 17
#define EFAULT 14
#define EFBIG 27
#define EILSEQ 88
#define EINTR 4
#define EINVAL 22
#define EIO 5
#define EISDIR 21
#define EMFILE 24
#define EMLINK 31
#define ENAMETOOLONG 36
#define ENFILE 23
#define ENODEV 19
#define ENOENT 2
#define ENOEXEC 8
#define ENOLCK 77
#define ENOMEM 12
#define ENOSPC 28
#define ENOSYS 38
#define ENOTDIR 20
#define ENOTEMPTY 39
#define ENOTTY 25
#define ENXIO 6
#define EPERM 1
#define EPIPE 32
#define EROFS 30
#define ESIGPARM 26
#define ESPIPE 29
#define ESRCH 3
#define EUNKNOWN 99
#define EXDEV 18
#else
#define ESIGPARM 36
#define EILSEQ 84
#endif

#ifdef __cplusplus
}
#endif

#endif
