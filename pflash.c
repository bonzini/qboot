#include <stddef.h>
#include "bios.h"
#include "pflash.h"
#include "stdio.h"

#define CLEAR_STATUS_CMD         0x50
#define READ_STATUS_CMD          0x70
#define QUERY_CMD                0x98
#define READ_ARRAY_CMD           0xff

static void *pflash_detect(uint8_t *top_addr)
{
	volatile uint8_t *p = top_addr;
	uint8_t save;
	int i, blocks, sector_len;

	/* The low byte of the address is part of the command, so it must be 0.  */
	if ((uintptr_t)p & 256)
		panic();

	p -= 256;
	for (i = 0; i < 256; i++)
		if (p[i] != CLEAR_STATUS_CMD)
			break;
	if (i == 256)
		return NULL;

	save = p[i];
	p[i] = CLEAR_STATUS_CMD;
	if (p[i] == CLEAR_STATUS_CMD) {
		/* behaves as RAM */
		p[i] = save;
		return NULL;
	}
	p[i] = READ_STATUS_CMD;
	if (p[i] != 0) {
		/* doesn't behave as flash */
		return NULL;
	}

	/* 0x2d-0x2e: blocks_per_device - 1, little endian */
	/* 0x2f-0x30: sector_len / 256 */
	p[i] = QUERY_CMD;
	blocks = p[0x2d] + (p[0x2e] << 8) + 1;
	sector_len = (p[0x2f] + (p[0x30] << 8)) << 8;

	p[i] = READ_ARRAY_CMD;
	return top_addr - blocks * sector_len;
}

void *pflash_base(int n, size_t *size)
{
	uint8_t *top = NULL;
	uint8_t *prev;
	while (n-- >= 0) {
		prev = top;
		top = pflash_detect(top);
		if (!top)
			return NULL;
		*size = prev - top;
	}
	return top;
}
