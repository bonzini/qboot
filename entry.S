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
	orb	$X86_EFLAGS_CF, 0x4(%esp)

	IRET
ENTRY_END(bios_intfake)

/*
 * int 10 - video - service
 */
ENTRY(bios_int10)
	andb	$~X86_EFLAGS_CF, 0x4(%esp)
	SAVE_BIOSREGS

	movl		%esp, %eax
	/* this is way easier than doing it in assembly */
	/* just push all the regs and jump to a C handler */
	call	int10_handler

	RESTORE_BIOSREGS

	IRET
ENTRY_END(bios_int10)

ENTRY(bios_int15)
	andb	$~X86_EFLAGS_CF, 0x4(%esp)
	SAVE_BIOSREGS

	movl	%esp, %eax
	call	int15_handler

	RESTORE_BIOSREGS

	IRET
ENTRY_END(bios_int15)

	.code32
ENTRY(pcibios_entry)
	clc
	pushfl
	SAVE_BIOSREGS

	movl	%esp, %eax
	call	pcibios_handler

	RESTORE_BIOSREGS
	popfl
	lretl
ENTRY_END(pcibios_entry)

ENTRY(bios32_entry)
	pushfl
	testl	%ebx, %ebx	   /* BIOS32 service directory? */
	jnz	2f
	cmp	$0x49435024, %eax  /* "$PCI"? */
	movb	$0x80, %al	   /* service not present */
	jne	1f
	xorl	%ebx, %ebx	   /* fill in base/length/entry */
	movl	$(1 << 20), %ecx
	movl	$pcibios_entry, %edx
	movb	$0x00, %al	   /* service present */
1:
	popfl
	lretl
2:
	movb	$0x81, %al	   /* unimplemented function */
	popfl
	lretl
ENTRY_END(bios32_entry)

ENTRY(pic_base)
	call 1f
2:
	ret
1:
	popl	%eax
	pushl	%eax
	subl	$2b, %eax
	ret			   /* return to 2b */
ENTRY_END(pic_base)
