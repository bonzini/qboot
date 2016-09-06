#include <inttypes.h>
#include "string.h"
#include "bios.h"

static uint8_t *fseg_base = &edata;
static uint8_t *malloc_top = &stext;

void *malloc(int n)
{
	malloc_top -= (n + 15) & -16;
	return malloc_top;
}

void *malloc_fseg(int n)
{
	void *p = fseg_base;
	fseg_base += (n + 15) & -16;
	return p;
}
