#include "bios.h"
#include "segment.h"

#include "processor-flags.h"
#include "e820.h"

static inline void set_fs(uint16_t seg)
{
	asm volatile("movw %0,%%fs" : : "rm" (seg));
}

static inline uint8_t rdfs8(unsigned long addr)
{
	uint8_t v;

	asm volatile("addr32 movb %%fs:%1,%0" : "=q" (v) : "m" (*(uint8_t *)addr));

	return v;
}

static inline uint32_t rdfs32(unsigned long addr)
{
	uint32_t v;

	asm volatile("addr32 movl %%fs:%1,%0" : "=q" (v) : "m" (*(uint32_t *)addr));

	return v;
}

bioscall void e820_query_map(struct biosregs *regs)
{
	struct e820map *e820;
	uint32_t map_size;
	uint16_t fs_seg;
	uint32_t ndx;

	e820		= (struct e820map *)E820_MAP_START;
	fs_seg		= flat_to_seg16(E820_MAP_START);
	set_fs(fs_seg);

	ndx		= regs->ebx;

	map_size	= rdfs32(flat_to_off16((uint32_t)&e820->nr_map, fs_seg));

	if (ndx < map_size) {
		uint32_t start;
		unsigned int i;
		uint8_t *p;

		fs_seg	= flat_to_seg16(E820_MAP_START);
		set_fs(fs_seg);

		start	= (uint32_t)&e820->map[ndx];

		p	= (void *) regs->edi;

		for (i = 0; i < sizeof(struct e820entry); i++)
			*p++	= rdfs8(flat_to_off16(start + i, fs_seg));
	}

	regs->eax	= SMAP;
	regs->ecx	= sizeof(struct e820entry);
	regs->ebx	= ++ndx;

	/* Clear CF to indicate success.  */
	regs->eflags	&= ~X86_EFLAGS_CF;

	if (ndx >= map_size)
		regs->ebx	= 0;	/* end of map */
}
