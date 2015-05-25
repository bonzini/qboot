#include <stdbool.h>
#include "bios.h"
#include "stdio.h"
#include "e820.h"
#include "pci.h"
#include "string.h"
#include "segment.h"
#include "fw_cfg.h"
#include "pflash.h"

#define PCI_VENDOR_ID_INTEL		0x8086
#define PCI_DEVICE_ID_INTEL_82441	0x1237
#define PCI_DEVICE_ID_INTEL_Q35_MCH     0x29c0

#define I440FX_PAM0                     0x59
#define Q35_HOST_BRIDGE_PAM0            0x90

static void make_bios_writable(void)
{
	// NOTE: this runs from ROM at 0xFFFF0000, so it is not possible to use any
	// static data.

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

	// Now go to the F-segment: we need to move away from flash area
	// in order to probe CBFS!
	asm("ljmp $0x8, $1f; 1:");
}

static void set_realmode_int(int vec, void *p)
{
	uint16_t *realmode_idt = (uint16_t *) 0;
	realmode_idt[vec * 2] = flat_to_off16((uintptr_t) p);
	realmode_idt[vec * 2 + 1] = flat_to_seg16((uintptr_t) p);
}

static void setup_pic(void)
{
    /* Send ICW1 (select OCW1 + will send ICW4) */
    outb(0x20, 0x11);
    outb(0xa0, 0x11);
    /* Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15) */
    outb(0x21, 8);
    outb(0xa1, 0x70);
    /* Send ICW3 (cascaded pic ids) */
    outb(0x21, 0x04);
    outb(0xa1, 0x02);
    /* Send ICW4 (enable 8086 mode) */
    outb(0x21, 0x01);
    outb(0xa1, 0x01);
    /* Mask all irqs (except cascaded PIC2 irq) */
    outb(0x21, ~(1 << 2));
    outb(0xa1, ~0);
}

static void setup_idt(void)
{
	int i;
	for (i = 0; i < 0x1f; i++)
		set_realmode_int(i, bios_intfake);
	for (i = 8; i < 16; i++)
		set_realmode_int(i, bios_irq);
	for (i = 0x70; i < 0x78; i++)
		set_realmode_int(i, bios_irq);
	set_realmode_int(0x10, bios_int10);
	set_realmode_int(0x15, bios_int15);
}

/* Top of memory below 4GB.  */
uint32_t lowmem;

static void extract_e820(void)
{
	int id = fw_cfg_file_id("etc/e820");
	uint32_t size;
	int nr_map;
	struct e820map *e820;
	int i;

	if (id == -1)
		panic();

	size = fw_cfg_file_size(id);
	nr_map = size / sizeof(e820->map[0]) + 4;
	fw_cfg_file_select(id);

	e820 = malloc(offsetof(struct e820map, map[nr_map]));
	e820->nr_map = nr_map;
	e820->map[0] = (struct e820entry)
		{ .addr = 0, .size = 639 * 1024, .type = E820_RAM }; /* low RAM */
	e820->map[1] = (struct e820entry)
		{ .addr = 639 * 1024, .size = 1024, .type = E820_RESERVED }; /* EBDA */
	e820->map[2] = (struct e820entry)
		{ .addr = 0xd0000, .size = 128 * 1024, .type = E820_NVS }; /* ACPI tables */
	e820->map[3] = (struct e820entry)
		{ .addr = 0xf0000, .size = 64 * 1024, .type = E820_RESERVED }; /* firmware */
	fw_cfg_read(&e820->map[4], size);
	for (i = 4; i < e820->nr_map; i++)
		if (e820->map[i].addr == 0) {
			lowmem = e820->map[i].size;
			e820->map[i].addr = 1024 * 1024;
			e820->map[i].size -= 1024 * 1024;
			break;
		}

	e820_seg = ((uintptr_t) e820) >> 4;
}

static bool detect_cbfs_and_boot(void)
{
	size_t sz;
	void *base = pflash_base(1, &sz);

	if (!base)
		return false;

	return boot_from_cbfs(base, sz);
}

int main(void)
{
	make_bios_writable();
	setup_pic();
	setup_idt();
	fw_cfg_setup();
	extract_acpi();
	extract_e820();
	// extract_smbios();
	if (!detect_cbfs_and_boot())
		boot_from_fwcfg();
	panic();
}
