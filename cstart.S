.code16gcc
#include "assembly.h"
.section .init
ENTRY(pm_entry)
	xor %ax, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	mov $0x7c00, %sp

	mov %cr0, %eax
	and $~((1 << 30) | (1 << 29)), %eax   # clear CD and NW
	or $1, %al
	mov %eax, %cr0
	lgdtl %cs:0xff80 + gdt32_descr - pm_entry
	ljmpl $8, $0xffffff80 + 2f - pm_entry
2:
	.code32
	mov $16, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	ljmp $8, $0xffff0000

gdt32:
	.quad 0
	.quad 0x00cf9b000000ffff // flat 32-bit code segment
	.quad 0x00cf93000000ffff // flat 32-bit data segment
	.quad 0x000f9b0f0000ffff // 64K 16-bit code segment at 0xF0000
	.quad 0x000f93000000ffff // 64K 16-bit data segment at 0x0
gdt32_end:

gdt32_descr:
	.word gdt32_end - gdt32 - 1
	.long 0xffffff80 + gdt32 - pm_entry
ENTRY_END(pm_entry)

	.code16gcc
	.section .resetvector
	jmp	pm_entry

