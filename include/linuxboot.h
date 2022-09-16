#ifndef BIOS_LINUXBOOT_H
#define BIOS_LINUXBOOT_H 1

#include <stdbool.h>

struct linuxboot_args {
	/* Output */
	void *setup_addr, *cmdline_addr, *kernel_addr, *initrd_addr;
	uint32_t setup_size, kernel_size;

	/* Input */
	uint32_t cmdline_size, vmlinuz_size, initrd_size;
};

bool parse_bzimage(struct linuxboot_args *args);
void boot_bzimage(struct linuxboot_args *args);

#endif
