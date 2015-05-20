/*
 * Our pretty trivial BIOS emulation
 */

#include "assembly.h"
#include "processor-flags.h"

	.org 0
	.code16gcc

/*
 * handy BIOS macros
 */

/* If you change these macros, remember to update 'struct biosregs' */
.macro SAVE_BIOSREGS
	pushl	%fs
	pushl	%es
	pushl	%ds
	pushl	%edi
	pushl	%esi
	pushl	%ebp
	pushl	%esp
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%eax
.endm

.macro RESTORE_BIOSREGS
	popl	%eax
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esp
	popl	%ebp
	popl	%esi
	popl	%edi
	popl	%ds
	popl	%es
	popl	%fs
.endm

ENTRY(bios_irq)
	pushw	%ax
	mov	$0x20, %al
	out	%al, $0x20
	popw	%ax
	IRET
ENTRY_END(bios_irq)

/*
 * fake interrupt handler, nothing can be faster ever
 */
ENTRY(bios_intfake)
	/*
	 * Set CF to indicate failure. We don't want callers to think that the
	 * interrupt handler succeeded and then treat the return values in
	 * registers as valid data.
	 */
	orl	$X86_EFLAGS_CF, 0x4(%esp)

	IRET
ENTRY_END(bios_intfake)

/*
 * int 10 - video - service
 */
ENTRY(bios_int10)
	SAVE_BIOSREGS

	movl		%esp, %eax
	/* this is way easier than doing it in assembly */
	/* just push all the regs and jump to a C handler */
	call	int10_handler

	RESTORE_BIOSREGS

	/* Clear CF to indicate success.  */
	andl	$~X86_EFLAGS_CF, 0x4(%esp)

	IRET
ENTRY_END(bios_int10)

ENTRY(bios_int15)
	SAVE_BIOSREGS

	movl	%esp, %eax
	call	int15_handler

	RESTORE_BIOSREGS

	IRET
ENTRY_END(bios_int15)
