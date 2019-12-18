#ifndef BIOS_H_
#define BIOS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * When interfacing with assembler code we need to be sure how
 * arguments are passed in real mode.
 */
#define bioscall __attribute__((regparm(3)))

#ifndef __ASSEMBLER__

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
	uint16_t			ip;
	uint16_t			cs;
	uint16_t			eflags;
} __attribute__((packed));

/*
 * BIOS32 is called via a far call, so eflags is pushed by our
 * entry point and lies below CS:EIP.  We do not include CS:EIP
 * at all in this struct.
 */
struct bios32regs {
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
	uint32_t			eflags;
} __attribute__((packed));

extern bioscall void int10_handler(struct biosregs *regs);
extern bioscall void int15_handler(struct biosregs *regs);
extern bioscall void e820_query_map(struct biosregs *regs);
extern bioscall void pcibios_handler(struct bios32regs *regs);

extern void bios_intfake(void);
extern void bios_irq(void);
extern void bios_int10(void);
extern void bios_int15(void);
extern void bios32_entry(void);

extern uint32_t pic_base(void);

extern void setup_pci(void);
extern bool setup_hw(void);
extern bool setup_mmconfig(void);
extern void setup_mptable(void);
extern void extract_acpi(void);
extern void boot_from_fwcfg(void);

extern uint8_t max_bus;
extern uint16_t e820_seg;
extern uint32_t lowmem;

extern uint8_t stext;
extern uint8_t edata;
extern uint8_t sinit;
extern uint8_t einit;

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

static inline void __attribute__((noreturn)) panic(void)
{
	asm volatile("cli; hlt");
	for(;;);
}

#endif

#endif /* BIOS_H_ */
