#ifndef BIOS_STRING_H
#define BIOS_STRING_H

#include <stddef.h>
#include <stdint.h>

unsigned long strlen(const char *buf);
char *strcat(char *dest, const char *src);
char *strcpy(char *dest, const char *src);
int strcmp(const char *a, const char *b);
char *strchr(const char *s, int c);
char *strstr(const char *s1, const char *s2);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memchr(const void *s, int c, size_t n);
uint8_t csum8(uint8_t *buf, uint32_t len);

static inline void *memset(void *s, int c, size_t n)
{
	return __builtin_memset(s, c, n);
}

static inline void *memcpy(void *dest, const void *src, size_t n)
{
	return __builtin_memcpy(dest, src, n);
}

void *malloc_align(int n, int align);
void *malloc_fseg_align(int n, int align);

static inline void *malloc(int n)
{
	return malloc_align(n, 16);
}

static inline void *malloc_fseg(int n)
{
	return malloc_fseg_align(n, 16);
}
#endif
