#ifndef _PCI_H
#define _PCI_H

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

#define PCI_VENDOR_ID 0

#endif
