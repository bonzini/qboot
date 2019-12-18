#include "bios.h"
#include "ioport.h"
#include "pci.h"
#include "string.h"

static uint16_t addend;
static uint8_t bus, bridge_head;
static bool use_i440fx_routing;
static int bridge_count;
uint8_t max_bus;

static void do_setup_pci_bus(void);

static void pci_foreach(void(*fn)(uint32_t bdf, uint32_t id, uint8_t type))
{
	int d, f;
	for (d = 0; d < 32; d++) {
		for (f = 0; f < 8; f++) {
			uint32_t bdf = (bus * 256) + (d * 8) + f;
			uint32_t id = pci_config_readl(bdf, PCI_VENDOR_ID);
			uint16_t vendor;
			uint8_t type;

			/* 0x0000 or 0xFFFF?  Skip.  */
			vendor = id & 0xFFFF;
			if ((uint16_t)(vendor + 1) <= 1) {
				if (f == 0)
					break;
				else
					continue;
			}

			type = pci_config_readb(bdf, PCI_HEADER_TYPE);
			fn(bdf, id, type);

			if (f == 0 && !(type & PCI_HEADER_TYPE_MULTI_FUNCTION))
				break;
		}
	}
}

static void do_setup_pci_irq(uint32_t bdf, int pin)
{
	int dev = (bdf >> 3) & 0x1f;
	int lnk, irq;

	irq = pci_config_readb(bdf, PCI_INTERRUPT_LINE);
	if (irq != 0)
		return;

	lnk = addend + pin;
	if (use_i440fx_routing)
		lnk += dev - 1;
	else {
		/* Q35 devices 25-31 all use LNKA.  Devices 0-24 have
		 * a slightly different mapping.
		 */
		if (dev <= 24)
			lnk += dev;
	}
	lnk &= 3;

	irq = lnk & 2 ? 11 : 10;
	pci_config_writeb(bdf, PCI_INTERRUPT_LINE, irq);
}

static void do_setup_pci(uint32_t bdf, uint32_t id, uint8_t type)
{
	uint16_t class;
	uint8_t pin;

	pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
	if (pin != 0)
		do_setup_pci_irq(bdf, pin);

	if (type & PCI_HEADER_TYPE_BRIDGE) {
		uint32_t ctl;

		ctl = pci_config_readw(bdf, PCI_BRIDGE_CONTROL);
		pci_config_writew(bdf, PCI_BRIDGE_CONTROL,
				  ctl | PCI_BRIDGE_CTL_SERR);
	}

	class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
	switch (class) {
	case PCI_CLASS_STORAGE_IDE:
		pci_config_writel(bdf, 0x10, 0x1f0);
		pci_config_writel(bdf, 0x14, 0x3f4);
		pci_config_writel(bdf, 0x18, 0x170);
		pci_config_writel(bdf, 0x1c, 0x374);
		if (id == (PCI_VENDOR_ID_INTEL | (PCI_DEVICE_ID_INTEL_82371SB_1 << 16))
		    || id == (PCI_VENDOR_ID_INTEL | (PCI_DEVICE_ID_INTEL_82371AB << 16))) {
			/* Enable IDE0 and IDE1.  */
			pci_config_writew(bdf, 0x40, 0x8000);
			pci_config_writew(bdf, 0x42, 0x8000);
		}
		break;

	case PCI_CLASS_BRIDGE_PCI:
		pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
		/* prevent accidental access to unintended devices */
		pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
		/*
		 * Insert at the head of a linked list of bridges.
		 * do_setup_pci_bus will use it later to initialize secondary
		 * buses with a recursive call.
		 */
		pci_config_writeb(bdf, PCI_SECONDARY_BUS, bridge_head);
		bridge_head = (uint8_t)(bdf & 0xFF);
		bridge_count++;
		break;
	}
}

static void do_setup_pci_bus(void)
{
	uint8_t save_bus, next_head;
	int i;

	bridge_head = 0xFF;
	bridge_count = 0;

	/* Discover all PCI devices and block bridges */
	pci_foreach(do_setup_pci);

	next_head = bridge_head;
	save_bus = bus;

	/* Configure bridges on this bus and recursively setup new busses */
	for (i = bridge_count; i > 0; i--) {
		uint32_t bdf = (save_bus * 256) + next_head;

		next_head = pci_config_readb(bdf, PCI_SECONDARY_BUS);

		bus = ++max_bus;
		pci_config_writeb(bdf, PCI_SECONDARY_BUS, bus);
		pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

		/* Add PCI bridge device id for the recursive call.  */
		addend += (bdf >> 3) & 0x1f;
		do_setup_pci_bus();
		addend -= (bdf >> 3) & 0x1f;

		pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, max_bus);
	}
}

void setup_bios32(void)
{
	char *bios32 = malloc_fseg_align(16, 16);
	void *bios32_entry_ = &bios32_entry;
	int i;

	memcpy(bios32, "_32_", 4);
	memcpy(bios32 + 4, &bios32_entry_, 4);
	bios32[8] = 0;
	bios32[9] = 1;
	memset(bios32 + 10, 0, 6);
	for (i = 0; i <= 9; i++)
		bios32[10] -= bios32[i];
}

void setup_pci(void)
{
	const int bdf = 0;

	uint32_t id = pci_config_readl(bdf, 0);
	if (id == (PCI_VENDOR_ID_INTEL | (PCI_DEVICE_ID_INTEL_82441 << 16)))
		use_i440fx_routing = true;
	else if (id == (PCI_VENDOR_ID_INTEL | (PCI_DEVICE_ID_INTEL_Q35_MCH << 16)))
		use_i440fx_routing = false;
	else
		panic();

	do_setup_pci_bus();
	setup_bios32();
}
