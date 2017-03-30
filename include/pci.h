#ifndef BIOS_PCI_H
#define BIOS_PCI_H

#include "ioport.h"

static inline void pci_config_writel(uint16_t bdf, uint32_t addr, uint32_t val)
{
	outl(0xcf8, 0x80000000 | (bdf << 8) | (addr & 0xfc));
	outl(0xcfc, val);
}

static inline void pci_config_writew(uint16_t bdf, uint32_t addr, uint16_t val)
{
	outl(0xcf8, 0x80000000 | (bdf << 8) | (addr & 0xfc));
	outw(0xcfc | (addr & 2), val);
}

static inline void pci_config_writeb(uint16_t bdf, uint32_t addr, uint8_t val)
{
	outl(0xcf8, 0x80000000 | (bdf << 8) | (addr & 0xfc));
	outb(0xcfc | (addr & 3), val);
}

static inline uint32_t pci_config_readl(uint16_t bdf, uint32_t addr)
{
	outl(0xcf8, 0x80000000 | (bdf << 8) | (addr & 0xfc));
	return inl(0xcfc);
}

static inline uint16_t pci_config_readw(uint16_t bdf, uint32_t addr)
{
	outl(0xcf8, 0x80000000 | (bdf << 8) | (addr & 0xfc));
	return inw(0xcfc | (addr & 2));
}

static inline uint8_t pci_config_readb(uint16_t bdf, uint32_t addr)
{
	outl(0xcf8, 0x80000000 | (bdf << 8) | (addr & 0xfc));
	return inb(0xcfc | (addr & 3));
}

#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND 0x04
#define PCI_CLASS_DEVICE 0x0a
#define PCI_HEADER_TYPE 0x0e
#define PCI_PRIMARY_BUS 0x18
#define PCI_SECONDARY_BUS 0x19
#define PCI_SUBORDINATE_BUS 0x1a
#define PCI_INTERRUPT_LINE 0x3c
#define PCI_INTERRUPT_PIN 0x3d
#define PCI_BRIDGE_CONTROL 0x3e

/* PCI_COMMAND */
#define PCI_COMMAND_DIS_INTX 0x400

/* PCI_CLASS_DEVICE */
#define PCI_CLASS_STORAGE_IDE 0x0101
#define PCI_CLASS_BRIDGE_PCI 0x0604

/* PCI_HEADER_TYPE */
#define PCI_HEADER_TYPE_BRIDGE 1
#define PCI_HEADER_TYPE_MULTI_FUNCTION 0x80

/* PCI_BRIDGE_CONTROL */
#define PCI_BRIDGE_CTL_SERR 0x02

/* PCI_VENDOR_ID / PCI_DEVICE_ID */
#define PCI_VENDOR_ID_INTEL		0x8086
#define PCI_DEVICE_ID_INTEL_82441	0x1237
#define PCI_DEVICE_ID_INTEL_Q35_MCH     0x29c0
#define PCI_DEVICE_ID_INTEL_82371SB_1	0x7010
#define PCI_DEVICE_ID_INTEL_82371AB	0x7111

#define PCIE_MMCONFIG_BASE 0xb0000000
#define PCIE_MMCONFIG_SIZE (256 * 1024 * 1024)

#endif
