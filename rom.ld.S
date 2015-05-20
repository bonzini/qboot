OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)

SECTIONS {
	.text 0	: {
		*(.text)
		. = 0xfff0 ;
		*(.fixedaddr._fff0)
		. = 0x10000 ;
	}

	/DISCARD/ : {
		*(.debug*)
		*(.data)
		*(.bss)
		*(.eh_frame*)
	}
}

