#include "string.h"

unsigned long strlen(const char *buf)
{
    unsigned long len = 0;

    while (*buf++)
	++len;
    return len;
}

char *strcat(char *dest, const char *src)
{
    char *p = dest;

    while (*p)
	++p;
    while ((*p++ = *src++) != 0)
	;
    return dest;
}

char *strcpy(char *dest, const char *src)
{
    *dest = 0;
    return strcat(dest, src);
}

int strcmp(const char *a, const char *b)
{
    while (*a == *b) {
	if (*a == '\0') {
	    break;
	}
	++a, ++b;
    }
    return *a - *b;
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c)
	if (*s++ == '\0')
	    return NULL;
    return (char *)s;
}

char *strstr(const char *s1, const char *s2)
{
    size_t l1, l2;

    l2 = strlen(s2);
    if (!l2)
	return (char *)s1;
    l1 = strlen(s1);
    while (l1 >= l2) {
	l1--;
	if (!memcmp(s1, s2, l2))
	    return (char *)s1;
	s1++;
    }
    return NULL;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *a = s1, *b = s2;
    int ret = 0;

    while (n--) {
	ret = *a - *b;
	if (ret)
	    break;
	++a, ++b;
    }
    return ret;
}

void *memmove(void *dest, const void *src, size_t n)
{
    const unsigned char *s = src;
    unsigned char *d = dest;

    if (d <= s) {
	while (n--)
	    *d++ = *s++;
    } else {
	d += n, s += n;
	while (n--)
	    *--d = *--s;
    }
    return dest;
}

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *str = s, chr = (unsigned char)c;

    while (n--)
	if (*str++ == chr)
	    return (void *)(str - 1);
    return NULL;
}

long atol(const char *ptr)
{
    long acc = 0;
    const char *s = ptr;
    int neg, c;

    while (*s == ' ' || *s == '\t')
        s++;
    if (*s == '-'){
        neg = 1;
        s++;
    } else {
        neg = 0;
        if (*s == '+')
            s++;
    }

    while (*s) {
        if (*s < '0' || *s > '9')
            break;
        c = *s - '0';
        acc = acc * 10 + c;
        s++;
    }

    if (neg)
        acc = -acc;

    return acc;
}

uint8_t csum8(uint8_t *buf, uint32_t len)
{
	uint32_t s = 0;
	while (len-- > 0)
		s += *buf++;
	return s;
}
