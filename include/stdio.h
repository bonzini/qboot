#ifndef BIOS_STDIO_H
#define BIOS_STDIO_H 1

#include <stdarg.h>

extern int puts(const char *s);
extern int printf(const char *fmt, ...);
extern int snprintf(char *buf, int size, const char *fmt, ...);
extern int vsnprintf(char *buf, int size, const char *fmt, va_list va);

#endif
