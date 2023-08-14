
#include "nobuild_log.h"

#include <stdlib.h>

void nobuild__vlog(FILE *stream, const char *tag, const char *fmt, va_list args)
{
    fprintf(stream, "[%s] ", tag);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
}

void VLOG(FILE *stream, const char *tag, const char *fmt, va_list args)
{
    WARN("This function is deprecated.");
    nobuild__vlog(stream, tag, fmt, args);
}

void info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nobuild__vlog(stderr, "INFO", fmt, args);
    va_end(args);
}

void warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nobuild__vlog(stderr, "WARN", fmt, args);
    va_end(args);
}

void erro(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nobuild__vlog(stderr, "ERRO", fmt, args);
    va_end(args);
}

void panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nobuild__vlog(stderr, "ERRO", fmt, args);
    va_end(args);
    exit(1);
}

void todo(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nobuild__vlog(stderr, "TODO", fmt, args);
    va_end(args);
    exit(1);
}

void todo_safe(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nobuild__vlog(stderr, "TODO", fmt, args);
    va_end(args);
}
