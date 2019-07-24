asm(".code16gcc");
#include <stddef.h>
#include "bios.h"
#include "segment.h"
#include "ioport.h"
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

	asm volatile("addr32 movl %%fs:%1,%0" : "=r" (v) : "m" (*(uint32_t *)addr));

	return v;
}

static inline uint16_t rdcs16(void *p)
{
	uint32_t addr = ((uintptr_t) p) & 65535;
	uint16_t v;

	asm volatile("addr32 movw %%cs:%1,%0" : "=r" (v) : "m" (*(uint32_t *)addr));

	return v;
}

uint16_t e820_seg;

bioscall void e820_query_map(struct biosregs *regs)
{
	uint32_t map_size;
	uint32_t ndx;

	set_fs(rdcs16(&e820_seg));

	ndx		= regs->ebx;

	map_size	= rdfs32(offsetof(struct e820map, nr_map));

	if (ndx < map_size) {
		uint32_t start;
		unsigned int i;
		uint8_t *p;

		start	= offsetof(struct e820map, map[ndx]);

		p	= (void *) regs->edi;

		for (i = 0; i < sizeof(struct e820entry); i++)
			*p++	= rdfs8(start + i);
	}

	regs->eax	= SMAP;
	regs->ecx	= sizeof(struct e820entry);
	regs->ebx	= ++ndx;

	if (ndx >= map_size)
		regs->ebx	= 0;	/* end of map */
}

bioscall void int15_handler(struct biosregs *regs)
{
	switch (regs->eax) {
	case 0xe820:
		e820_query_map(regs);
		break;
	default:
		/* Set CF to indicate failure.  */
		regs->eflags	|= X86_EFLAGS_CF;
		break;
	}
}
/*
 * It's probably much more useful to make this print to the serial
 * line rather than print to a non-displayed VGA memory
 */
static inline void int10_putchar(struct biosregs *args)
{
	uint8_t al = args->eax & 0xFF;

	outb(0x3f8, al);
}

#define VBE_STATUS_OK		0x004F
#define VBE_STATUS_FAIL		0x014F

static void int10_vesa(struct biosregs *args)
{
	args->eax = VBE_STATUS_FAIL;
}

bioscall void int10_handler(struct biosregs *args)
{
	uint8_t ah;

	ah = (args->eax & 0xff00) >> 8;

	switch (ah) {
	case 0x0e:
		int10_putchar(args);
		break;
	case 0x4f:
		int10_vesa(args);
		break;
	}

}
