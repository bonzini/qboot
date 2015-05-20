#include "bios.h"

#include "processor-flags.h"

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
