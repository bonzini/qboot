#ifndef BIOS_H_
#define BIOS_H_

#include <inttypes.h>

/*
 * X86-32 Memory Map (typical)
 *					start      end
 * Real Mode Interrupt Vector Table	0x00000000 0x000003FF
 * BDA area				0x00000400 0x000004FF
 * Conventional Low Memory		0x00000500 0x0009FBFF
 * EBDA area				0x0009FC00 0x0009FFFF
 * VIDEO RAM				0x000A0000 0x000BFFFF
 * VIDEO ROM (BIOS)			0x000C0000 0x000C7FFF
 * ROMs & unus. space (mapped hw & misc)0x000C8000 0x000EFFFF 160 KiB (typically)
 * Motherboard BIOS			0x000F0000 0x000FFFFF
 * Extended Memory			0x00100000 0xFEBFFFFF
 * Reserved (configs, ACPI, PnP, etc)	0xFEC00000 0xFFFFFFFF
 */

#define REAL_MODE_IVT_BEGIN		0x00000000
#define REAL_MODE_IVT_END		0x000003ff

#define BDA_START			0x00000400
#define BDA_END				0x000004ff

#define EBDA_START			0x0009fc00
#define EBDA_END			0x0009ffff

#define E820_MAP_START			EBDA_START

/*
 * When interfacing with assembler code we need to be sure how
 * arguments are passed in real mode.
 */
#define bioscall __attribute__((regparm(3)))

#ifndef __ASSEMBLER__

#include <linux/types.h>

struct biosregs {
	uint32_t			eax;
	uint32_t			ebx;
	uint32_t			ecx;
	uint32_t			edx;
	uint32_t			esp;
	uint32_t			ebp;
	uint32_t			esi;
	uint32_t			edi;
	uint32_t			ds;
	uint32_t			es;
	uint32_t			fs;
	uint32_t			eip;
	uint32_t			eflags;
};

extern bioscall void int10_handler(struct biosregs *regs);
extern bioscall void int15_handler(struct biosregs *regs);
extern bioscall void e820_query_map(struct biosregs *regs);

#endif

#endif /* BIOS_H_ */
