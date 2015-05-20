#ifndef BIOS_H_
#define BIOS_H_

#include <inttypes.h>

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
	uint32_t			eip;
	uint32_t			eflags;
};

extern bioscall void int10_handler(struct biosregs *regs);
extern bioscall void int15_handler(struct biosregs *regs);
extern bioscall void e820_query_map(struct biosregs *regs);

extern void bios_intfake(void);
extern void bios_irq(void);
extern void bios_int10(void);
extern void bios_int15(void);

extern void boot_linux(void);

extern struct e820map e820;

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

static inline void __attribute__((noreturn)) panic(void)
{
	asm volatile("cli; hlt");
	for(;;);
}

#endif

#endif /* BIOS_H_ */
