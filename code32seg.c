#include <stddef.h>
#include "bios.h"
#include "pci.h"
#include "processor-flags.h"

#define PCI_FUNC_NOT_SUPPORTED 0x81
#define PCI_BAD_VENDOR_ID      0x83
#define PCI_DEVICE_NOT_FOUND   0x86
#define PCI_BUFFER_TOO_SMALL   0x89

/*
 * The PCIBIOS handler must be position independent.  To read a flat pointer,
 * we use the instruction pointer to retrieve the address corresponding to
 * physical address 0 (i.e., what Linux calls PAGE_OFFSET).
 */

static inline void *from_flat_ptr(void *p)
{
	return p + pic_base();
}

#define FLAT_VAR(x)  (*(typeof(&(x))) from_flat_ptr(&(x)))

bioscall void pcibios_handler(struct bios32regs *args)
{
	switch (args->eax) {
		/* discovery */
	case 0xb101:
		args->eax = 0x01;
		args->ebx = 0x210;
		args->ecx = FLAT_VAR(max_bus);
		args->edx = 0x20494350;
		goto success;

		/* config space access */
	case 0xb108:
		args->ecx = pci_config_readb(args->ebx, args->edi);
		goto success;
	case 0xb109:
		args->ecx = pci_config_readw(args->ebx, args->edi);
		goto success;
	case 0xb10a:
		args->ecx = pci_config_readl(args->ebx, args->edi);
		goto success;
	case 0xb10b:
		pci_config_writeb(args->ebx, args->edi, args->ecx);
		goto success;
	case 0xb10c:
		pci_config_writew(args->ebx, args->edi, args->ecx);
		goto success;
	case 0xb10d:
		pci_config_writel(args->ebx, args->edi, args->ecx);
		goto success;

		/* find device id, find class code */
	case 0xb102:
	case 0xb103:
		args->eax &= ~0xff00;
		args->eax |= PCI_DEVICE_NOT_FOUND << 8;
		break;

	default:
		args->eax &= ~0xff00;
		args->eax |= PCI_FUNC_NOT_SUPPORTED << 8;
		break;
	}
	args->eflags |= X86_EFLAGS_CF;
	return;

success:
	/* On entry, CF=0 */
	args->eax &= ~0xff00; /* clear ah */
}
