#include <stdbool.h>
#include "bios.h"
#include "stdio.h"
#include "e820.h"
#include "string.h"
#include "segment.h"
#include "fw_cfg.h"
#include "pci.h"
#include "benchmark.h"

static void set_realmode_int(int vec, void *p)
{
	uint16_t *realmode_idt = (uint16_t *) 0;
	realmode_idt[vec * 2] = flat_to_off16((uintptr_t) p);
	realmode_idt[vec * 2 + 1] = flat_to_seg16((uintptr_t) p);
}

static void setup_idt(void)
{
	int i;
	for (i = 0; i < 0x100; i++)
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
struct e820map *e820;
static bool have_mmconfig;

static void extract_e820(void)
{
	int id = fw_cfg_file_id("etc/e820");
	uint32_t size;
	int nr_map;
	int i;

	if (id == -1)
		panic();

	size = fw_cfg_file_size(id);
	nr_map = size / sizeof(e820->map[0]) + 5;

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

	i = 4;
	if (have_mmconfig)
		e820->map[i++] = (struct e820entry)
			{ .addr = PCIE_MMCONFIG_BASE, .size = PCIE_MMCONFIG_SIZE, .type = E820_RESERVED };
	else
		nr_map--;

	fw_cfg_read_file(id, &e820->map[i], size);
	for (; i < e820->nr_map; i++)
		if (e820->map[i].addr == 0) {
			lowmem = e820->map[i].size;
			e820->map[i].addr = 1024 * 1024;
			e820->map[i].size -= 1024 * 1024;
			break;
		}

	e820_seg = ((uintptr_t) e820) >> 4;
}

int __attribute__ ((section (".text.startup"))) main(void)
{
	bool have_pci;
#ifdef BENCHMARK_HACK
	outb(FW_EXIT_PORT, FW_START);
#endif
	have_pci = setup_hw();

	// Only the 16-bit trampoline for vmlinuz and the 16-bit interrupt
	// handlers need to run from the F-segment, but keep things simple
	// and jump there.  From this point we can modify global variables.
	asm("ljmp $0x8, $1f; 1:");

	have_mmconfig = setup_mmconfig();
	if (have_pci) {
		setup_pci();
	}
	setup_idt();
	fw_cfg_setup();
	extract_acpi();
	extract_e820();
	setup_mptable();
	// extract_smbios();
	boot_from_fwcfg();
	panic();
}
