OUTPUT_ARCH(i386)

SECTIONS
{
    . = 1024K - 64K;
    .text : { *(.text.startup) *(.text) *(.text.*) }
    . = ALIGN(4K);
    .data : { *(.data) }
    . = ALIGN(16);
    .rodata : { *(.rodata) }
    . = ALIGN(16);
    .bss : { *(.bss) }
    . = ALIGN(4K);
    . = 1024K - 128;
    .init : {
	*(.init);
	. = 128 - 16;
        *(.resetvector);
	. = 128;
    }
}

ENTRY(main)

