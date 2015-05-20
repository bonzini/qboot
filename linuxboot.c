#include "bios.h"
#include "ioport.h"
#include "fw_cfg.h"

static void *_fw_cfg_read_blob(int faddr, int fsize, int fdata)
{
	void *addr;
	int length;

	fw_cfg_select(faddr);
	addr = (void *)fw_cfg_readl_le();
	fw_cfg_select(fsize);
	length = fw_cfg_readl_le();
	fw_cfg_select(fdata);
	fw_cfg_read(addr, length);
	return addr;
}

/* BX = address of data block
 * DX = cmdline_addr-setup_addr-16
 */
asm("pm16_boot_linux:"
	    ".code16;"
	    "mov $0, %eax; mov %eax, %cr0;"
	    "ljmpl $0xf000, $(1f - 0xf0000); 1:"
	    "mov %bx, %ds; mov %bx, %es;"
	    "mov %bx, %fs; mov %bx, %gs; mov %bx, %ss;"
	    "mov %dx, %sp;"
	    "add $0x20, %bx; pushw %bx;"    // push CS
	    "xor %eax, %eax; pushw %ax;"    // push IP
	    "xor %ebx, %ebx;"
	    "xor %ecx, %ecx;"
	    "xor %edx, %edx;"
	    "xor %edi, %edi;"
	    "xor %ebp, %ebp;"
	    "lret;"
	    ".code32");

void boot_linux(void)
{
	void *setup_addr, *cmdline_addr;

#define fw_cfg_read_blob(f) \
	_fw_cfg_read_blob(f##_ADDR, f##_SIZE, f##_DATA)

	setup_addr = fw_cfg_read_blob(FW_CFG_SETUP);
	cmdline_addr = fw_cfg_read_blob(FW_CFG_CMDLINE);
	fw_cfg_read_blob(FW_CFG_INITRD);
	fw_cfg_read_blob(FW_CFG_KERNEL);

	asm volatile(
	    "ljmp $0x18, $pm16_boot_linux - 0xf0000"
	    : :
	    "b" (((uintptr_t) setup_addr) >> 4),
	    "d" (cmdline_addr - setup_addr - 16));
        panic();
}
