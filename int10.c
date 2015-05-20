#include "bios.h"
#include "segment.h"

static inline void outb(unsigned short port, unsigned char val)
{
	asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
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
