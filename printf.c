#include "bios.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "ioport.h"

typedef struct pstream {
    char *buffer;
    int remain;
    int added;
} pstream_t;

typedef struct strprops {
    char pad;
    int npad;
} strprops_t;

static void addchar(pstream_t *p, char c)
{
    if (p->remain) {
	*p->buffer++ = c;
	--p->remain;
    }
    ++p->added;
}

int puts(const char *c)
{
    int n = 0;
    while (c[n])
        outb(0x3f8, c[n++]);
    return n;
}

void print_str(pstream_t *p, const char *s, strprops_t props)
{
    const char *s_orig = s;
    int npad = props.npad;

    if (npad > 0) {
	npad -= strlen(s_orig);
	while (npad > 0) {
	    addchar(p, props.pad);
	    --npad;
	}
    }

    while (*s)
	addchar(p, *s++);

    if (npad < 0) {
	props.pad = ' '; /* ignore '0' flag with '-' flag */
	npad += strlen(s_orig);
	while (npad < 0) {
	    addchar(p, props.pad);
	    ++npad;
	}
    }
}

static char digits[16] = "0123456789abcdef";

void print_int(pstream_t *ps, long n, int base, strprops_t props)
{
    char buf[sizeof(long) * 3 + 2], *p = buf;
    int s = 0, i;

    if (n < 0) {
	n = -n;
	s = 1;
    }

    while (n) {
	*p++ = digits[n % base];
	n /= base;
    }

    if (s)
	*p++ = '-';

    if (p == buf)
	*p++ = '0';

    for (i = 0; i < (p - buf) / 2; ++i) {
	char tmp;

	tmp = buf[i];
	buf[i] = p[-1-i];
	p[-1-i] = tmp;
    }

    *p = 0;

    print_str(ps, buf, props);
}

void print_unsigned(pstream_t *ps, unsigned long n, int base,
		    strprops_t props)
{
    char buf[sizeof(long) * 3 + 1], *p = buf;
    int i;

    while (n) {
	*p++ = digits[n % base];
	n /= base;
    }

    if (p == buf)
	*p++ = '0';

    for (i = 0; i < (p - buf) / 2; ++i) {
	char tmp;

	tmp = buf[i];
	buf[i] = p[-1-i];
	p[-1-i] = tmp;
    }

    *p = 0;

    print_str(ps, buf, props);
}

static int fmtnum(const char **fmt)
{
    const char *f = *fmt;
    int len = 0, num;

    if (*f == '-')
	++f, ++len;

    while (*f >= '0' && *f <= '9')
	++f, ++len;

    num = atol(*fmt);
    *fmt += len;
    return num;
}

int vsnprintf(char *buf, int size, const char *fmt, va_list va)
{
    pstream_t s;

    s.buffer = buf;
    s.remain = size - 1;
    s.added = 0;
    while (*fmt) {
	char f = *fmt++;
	int nlong = 0;
	strprops_t props;
	memset(&props, 0, sizeof(props));
	props.pad = ' ';

	if (f != '%') {
	    addchar(&s, f);
	    continue;
	}
    morefmt:
	f = *fmt++;
	if (f == '%') {
	    addchar(&s, '%');
	    continue;
	}
	if (f == 'c') {
            addchar(&s, va_arg(va, int));
	    continue;
	}
	if (f == '\0') {
	    --fmt;
	    continue;
	}
	if (f == '0') {
	    props.pad = '0';
	    ++fmt;
	    /* fall through */
	}
	if ((f >= '1' && f <= '9') || f == '-') {
	    --fmt;
	    props.npad = fmtnum(&fmt);
	    goto morefmt;
	}
	if (f == 'l') {
	    ++nlong;
	    goto morefmt;
	}
	if (f == 'd') {
	    switch (nlong) {
	    case 0:
		print_int(&s, va_arg(va, int), 10, props);
		break;
	    case 1:
		print_int(&s, va_arg(va, long), 10, props);
		break;
	    default:
		panic();
		break;
	    }
	    continue;
	}
	if (f == 'x') {
	    switch (nlong) {
	    case 0:
		print_unsigned(&s, va_arg(va, unsigned), 16, props);
		break;
	    case 1:
		print_unsigned(&s, va_arg(va, unsigned long), 16, props);
		break;
	    default:
		panic();
		break;
	    }
	    continue;
	}
	if (f == 'p') {
	    print_str(&s, "0x", props);
	    print_unsigned(&s, (unsigned long)va_arg(va, void *), 16, props);
	    continue;
	}
	if (f == 's') {
	    print_str(&s, va_arg(va, const char *), props);
	    continue;
	}
        addchar(&s, f);
    }
    *s.buffer = 0;
    ++s.added;
    return s.added;
}


int snprintf(char *buf, int size, const char *fmt, ...)
{
    va_list va;
    int r;

    va_start(va, fmt);
    r = vsnprintf(buf, size, fmt, va);
    va_end(va);
    return r;
}

int printf(const char *fmt, ...)
{
    va_list va;
    char buf[2000];
    int r;

    va_start(va, fmt);
    r = vsnprintf(buf, sizeof buf, fmt, va);
    va_end(va);
    puts(buf);
    return r;
}
