OUTPUT_ARCH(i386)

SECTIONS
{
    . = 1024K - 64K;
    stext = .;
    .text : { *(.text.startup) *(.text) *(.text.*) }
    . = ALIGN(16);
    .data : { *(.data) }
    . = ALIGN(16);
    .rodata : { *(.rodata) }
    . = ALIGN(16);
    .bss : { *(.bss) }
    . = ALIGN(16);
    edata = .;
    . = 1024K - 128;
    sinit = .;
    .init : {
	*(.init);
	. = 128 - 16;
        *(.resetvector);
	. = 128;
    }
    einit = .;
}

ENTRY(main)

