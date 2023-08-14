#pragma once

#ifndef _WIN32
#    include <sys/types.h>
typedef pid_t Pid;
typedef int Fd;
#else
#    define WIN32_MEAN_AND_LEAN
#    include <windows.h>
typedef HANDLE Pid;
typedef HANDLE Fd;
#endif

#ifndef NOBUILD_PRINTF_FORMAT
#	if defined(__GNUC__) || defined(__clang__)
#		// https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
#		define NOBUILD_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))
#	else
#		define NOBUILD_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#	endif
#endif

typedef struct {
    Fd read;
    Fd write;
} Pipe;

Pipe pipe_make(void);

Fd fd_open_for_read(const char *path);
Fd fd_open_for_write(const char *path);
size_t fd_read(Fd fd, void *buf, unsigned long count);
size_t fd_write(Fd fd, void *buf, unsigned long count);
int fd_printf(Fd fd, const char *fmt, ...) NOBUILD_PRINTF_FORMAT(2, 3);
void fd_close(Fd fd);

void pid_wait(Pid pid);
