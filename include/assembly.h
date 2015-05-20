#ifndef ASSEMBLY_H_
#define ASSEMBLY_H_

#define __ASSEMBLY__

#define BIOS2FLAT(x)	((x) - _start + 0xf0000)
#define BIOS2FLATROM(x)	((x) - _start + 0xffff0000)
_start = 0

#define ORG(x)	\
	.section .fixedaddr._##x

#define __ALIGN	.p2align 4, 0x90
#define ENTRY(name)	\
	__ALIGN;	\
	.globl name;	\
	name:

#define GLOBAL(name)	\
	.globl name;	\
	name:

#define ENTRY_END(name)	GLOBAL(name##_end)
#define END(name)	GLOBAL(name##_end)

/*
 * gas produces size override prefix with which
 * we are unhappy, lets make it hardcoded for
 * 16 bit mode
 */
#define IRET	.byte 0xcf

#endif /* ASSEMBLY_H_ */
