#ifndef BIOS_FW_CFG_H
#define BIOS_FW_CFG_H 1

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#define FW_CFG_SIGNATURE              0x00
#define FW_CFG_ID                     0x01
#define FW_CFG_UUID                   0x02
#define FW_CFG_RAM_SIZE               0x03
#define FW_CFG_NOGRAPHIC              0x04
#define FW_CFG_NB_CPUS                0x05
#define FW_CFG_MACHINE_ID             0x06
#define FW_CFG_KERNEL_ADDR            0x07
#define FW_CFG_KERNEL_SIZE            0x08
#define FW_CFG_KERNEL_CMDLINE         0x09
#define FW_CFG_INITRD_ADDR            0x0a
#define FW_CFG_INITRD_SIZE            0x0b
#define FW_CFG_BOOT_DEVICE            0x0c
#define FW_CFG_NUMA                   0x0d
#define FW_CFG_BOOT_MENU              0x0e
#define FW_CFG_MAX_CPUS               0x0f
#define FW_CFG_KERNEL_ENTRY           0x10
#define FW_CFG_KERNEL_DATA            0x11
#define FW_CFG_INITRD_DATA            0x12
#define FW_CFG_CMDLINE_ADDR           0x13
#define FW_CFG_CMDLINE_SIZE           0x14
#define FW_CFG_CMDLINE_DATA           0x15
#define FW_CFG_SETUP_ADDR             0x16
#define FW_CFG_SETUP_SIZE             0x17
#define FW_CFG_SETUP_DATA             0x18
#define FW_CFG_FILE_DIR               0x19
#define FW_CFG_FILE_FIRST             0x20
#define FW_CFG_ARCH_LOCAL             0x8000
#define FW_CFG_ACPI_TABLES            (FW_CFG_ARCH_LOCAL + 0)
#define FW_CFG_SMBIOS_ENTRIES         (FW_CFG_ARCH_LOCAL + 1)
#define FW_CFG_IRQ0_OVERRIDE          (FW_CFG_ARCH_LOCAL + 2)
#define FW_CFG_E820_TABLE             (FW_CFG_ARCH_LOCAL + 3)

#define FW_CFG_VERSION                0x01
#define FW_CFG_VERSION_DMA            0x02

#define FW_CFG_DMA_CTL_ERROR          0x01
#define FW_CFG_DMA_CTL_READ           0x02
#define FW_CFG_DMA_CTL_SKIP           0x04
#define FW_CFG_DMA_CTL_SELECT         0x08

#define FW_CFG_CTL                    0x510
#define FW_CFG_DATA                   0x511
#define FW_CFG_DMA_ADDR_HIGH          0x514
#define FW_CFG_DMA_ADDR_LOW           0x518

#include "ioport.h"

static inline void fw_cfg_select(uint16_t f)
{
	outw(FW_CFG_CTL, f);
}

static inline uint32_t fw_cfg_readb(void)
{
	return inb(FW_CFG_DATA);
}

static inline uint32_t fw_cfg_readw_be(void)
{
	uint32_t val;
	
	val = inb(FW_CFG_DATA);
	val = (val << 8) | inb(FW_CFG_DATA);
	return val;
}

static inline uint32_t fw_cfg_readw_le(void)
{
	uint32_t val;

	val = inb(FW_CFG_DATA);
	val = (inb(FW_CFG_DATA) << 8) | val;
	return val;
}

static inline uint32_t fw_cfg_readl_be(void)
{
	uint32_t val;
	
	val = inb(FW_CFG_DATA);
	val = (val << 8) | inb(FW_CFG_DATA);
	val = (val << 8) | inb(FW_CFG_DATA);
	val = (val << 8) | inb(FW_CFG_DATA);
	return val;
}

static inline uint32_t fw_cfg_readl_le(void)
{
	uint32_t val;

	val = inb(FW_CFG_DATA);
	val = (inb(FW_CFG_DATA) << 8) | val;
	val = (inb(FW_CFG_DATA) << 16) | val;
	val = (inb(FW_CFG_DATA) << 24) | val;
	return val;
}

static inline void fw_cfg_skip(int len)
{
	while (len--)
		inb(FW_CFG_DATA);
}

void fw_cfg_setup(void);
int fw_cfg_file_id(char *name);
uint32_t fw_cfg_file_size(int id);
void fw_cfg_file_select(int id);

void fw_cfg_read(void *buf, int len);
void fw_cfg_read_entry(int e, void *buf, int len);
void fw_cfg_dma(int control, void *buf, int len);
void fw_cfg_read_file(int e, void *buf, int len);

#endif
