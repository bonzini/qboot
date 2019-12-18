#include <stdint.h>
#include "string.h"
#include "bios.h"

static uint8_t *fseg_base = &edata;
static uint8_t *malloc_top = &stext;

void *malloc_align(int n, int align)
{
	malloc_top = (uint8_t *) ((uintptr_t)(malloc_top - n) & -align);
	return malloc_top;
}

void *malloc_fseg_align(int n, int align)
{
	void *p;
	fseg_base = (uint8_t *) (((uintptr_t)fseg_base + align - 1) & -align);
	p = fseg_base;
	fseg_base += n;
	return p;
}
