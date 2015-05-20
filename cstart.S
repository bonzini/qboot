.code16gcc
#include "assembly.h"
.section .init
ENTRY(pm_entry)
	xor %ax, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov $0x7c00, %sp

        mov %cr0, %eax
        or $1, %eax
        mov %eax, %cr0
        lgdtl %cs:0xff80 + gdt32_descr - pm_entry
        ljmpl $8, $0xffffff80 + 2f - pm_entry
2:
	.code32
	mov $16, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	ljmp $8, $0xffff0000

gdt32:
        .quad 0
        .quad 0x00cf9b000000ffff // flat 32-bit code segment
        .quad 0x00cf93000000ffff // flat 32-bit data segment
gdt32_end:

gdt32_descr:
        .word gdt32_end - gdt32 - 1
        .long 0xffffff80 + gdt32 - pm_entry
ENTRY_END(pm_entry)

	.code16gcc
	.section .resetvector
	jmp	pm_entry

