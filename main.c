#include "bios.h"
#include "pci.h"
#include "string.h"
#include "segment.h"

#define PCI_VENDOR_ID_INTEL		0x8086
#define PCI_DEVICE_ID_INTEL_82441	0x1237
#define PCI_DEVICE_ID_INTEL_Q35_MCH     0x29c0

#define I440FX_PAM0                     0x59
#define Q35_HOST_BRIDGE_PAM0            0x90

static void make_bios_writable(void)
{
	const int bdf = 0;
	const uint8_t *bios_start = (uint8_t *)0xffff0000;
	uint8_t *low_start = (uint8_t *)0xf0000;
	int pambase;

        uint32_t id = pci_config_readl(bdf, 0);
        if (id == (PCI_VENDOR_ID_INTEL | (PCI_DEVICE_ID_INTEL_82441 << 16)))
		pambase = I440FX_PAM0;
        else if (id == (PCI_VENDOR_ID_INTEL | (PCI_DEVICE_ID_INTEL_Q35_MCH << 16)))
		pambase = Q35_HOST_BRIDGE_PAM0;
        else
		panic();

	// Make ram from 0xc0000-0xf0000 read-write
	int i;
	for (i=0; i<6; i++) {
		int pam = pambase + 1 + i;
		pci_config_writeb(bdf, pam, 0x33);
	}

	// Make ram from 0xf0000-0x100000 read-write and shadow BIOS
	// We're still running from 0xffff0000
	pci_config_writeb(bdf, pambase, 0x30);
	memcpy(low_start, bios_start, 0x10000);
}

static void set_realmode_int(int vec, void *p)
{
	uint16_t *realmode_idt = (uint16_t *) 0;
	realmode_idt[vec * 2] = flat_to_off16((uintptr_t) p);
	realmode_idt[vec * 2 + 1] = flat_to_seg16((uintptr_t) p);
}

static void setup_idt(void)
{
	int i;
	for (i = 0; i < 0x1f; i++)
		set_realmode_int(i, bios_intfake);
	set_realmode_int(0x10, bios_int10);
	set_realmode_int(0x15, bios_int15);
}

int main(void)
{
	make_bios_writable();
	setup_idt();
	// extract_acpi();
	// extract_e820();
	// extract_smbios();
	// extract_kernel();
	// make_bios_readonly();
	// boot_linux();
	panic();
}
