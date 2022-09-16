#include "bios.h"
#include "e820.h"
#include "stdio.h"
#include "ioport.h"
#include "string.h"
#include "fw_cfg.h"
#include "bswap.h"
#include "linuxboot.h"
#include "memaccess.h"
#include "multiboot.h"
#include "benchmark.h"
#include "start_info.h"

extern struct hvm_start_info start_info;

struct fw_cfg_file {
	uint32_t size;
	uint16_t select;
	uint16_t unused;
	char name[56];
};

static int version;
static int filecnt;
static struct fw_cfg_file *files;

void fw_cfg_setup(void)
{
	int i, n;

	fw_cfg_select(FW_CFG_ID);
	version = fw_cfg_readl_le();

	fw_cfg_select(FW_CFG_FILE_DIR);
	n = fw_cfg_readl_be();
	filecnt = n;
	files = malloc_fseg(sizeof(files[0]) * n);

	fw_cfg_read(files, sizeof(files[0]) * n);
	for (i = 0; i < n; i++) {
		struct fw_cfg_file *f = &files[i];
		f->size = bswap32(f->size);
		f->select = bswap16(f->select);
	}
}

int filenamecmp(const char *a, const struct fw_cfg_file *f)
{
	int n = sizeof(f->name);
	const char *b = f->name;
	while (*a == *b) {
		if (*a == '\0') {
			break;
		}
		if (--n == 0) {
			return *a;
		}
		++a, ++b;
	}
	return *a - *b;
}

int fw_cfg_file_id(char *name)
{
	int i;

	for (i = 0; i < filecnt; i++)
		if (!filenamecmp(name, &files[i]))
			return i;

	return -1;
}

uint32_t fw_cfg_file_size(int id)
{
	if (id == -1)
		return 0;
	return files[id].size;
}

void fw_cfg_file_select(int id)
{
	fw_cfg_select(files[id].select);
}

void fw_cfg_read_file(int id, void *buf, int len)
{
	fw_cfg_read_entry(files[id].select, buf, len);
}

struct fw_cfg_dma_descriptor {
	uint32_t control;
	uint32_t length;
	uint64_t address;
} __attribute__((packed));

void fw_cfg_dma(int control, void *buf, int len)
{
	volatile struct fw_cfg_dma_descriptor dma;
	uint32_t dma_desc_addr;

	dma.control = bswap32(control);
	dma.length = bswap32(len);
	dma.address = bswap64((uintptr_t)buf);

	dma_desc_addr = (uint32_t)&dma;
	outl(FW_CFG_DMA_ADDR_LOW, bswap32(dma_desc_addr));
	while (bswap32(dma.control) & ~FW_CFG_DMA_CTL_ERROR) {
		asm("");
	}
}

void fw_cfg_read(void *buf, int len)
{
	if (version & FW_CFG_VERSION_DMA) {
		fw_cfg_dma(FW_CFG_DMA_CTL_READ, buf, len);
	} else {
		insb(buf, FW_CFG_DATA, len);
	}
}

void
fw_cfg_read_entry(int e, void *buf, int len)
{
	if (version & FW_CFG_VERSION_DMA) {
		int control;
		control = (e << 16);
		control |= FW_CFG_DMA_CTL_SELECT;
		control |= FW_CFG_DMA_CTL_READ;
		fw_cfg_dma(control, buf, len);
	} else {
		fw_cfg_select(e);
		insb(buf, FW_CFG_DATA, len);
	}
}

/* Multiboot trampoline.  QEMU does the ELF parsing.  */

static void boot_multiboot_from_fw_cfg(void)
{
	void *kernel_addr, *kernel_entry;
	struct mb_info *mb;
	struct mb_mmap_entry *mbmem;
	int i;
	uint32_t sz;

	fw_cfg_select(FW_CFG_KERNEL_SIZE);
	sz = fw_cfg_readl_le();
	if (!sz)
		panic();

	fw_cfg_select(FW_CFG_KERNEL_ADDR);
	kernel_addr = (void *) fw_cfg_readl_le();
	fw_cfg_read_entry(FW_CFG_KERNEL_DATA, kernel_addr, sz);

	fw_cfg_select(FW_CFG_INITRD_SIZE);
	sz = fw_cfg_readl_le();
	if (!sz)
		panic();

	fw_cfg_select(FW_CFG_INITRD_ADDR);
	mb = (struct mb_info *) fw_cfg_readl_le();
	fw_cfg_read_entry(FW_CFG_INITRD_DATA, mb, sz);

	mb->mem_lower = 639;
	mb->mem_upper = (lowmem - 1048576) >> 10;

	mb->mmap_length = 0;
	for (i = 0; i < e820->nr_map; i++) {
		mbmem = (struct mb_mmap_entry *) (mb->mmap_addr + mb->mmap_length);
		mbmem->size = sizeof(e820->map[i]);
		mbmem->base_addr = e820->map[i].addr;
		mbmem->length = e820->map[i].size;
		mbmem->type = e820->map[i].type;
		mb->mmap_length += sizeof(*mbmem);
	}

#ifdef BENCHMARK_HACK
	/* Exit just before getting to vmlinuz, so that it is easy
	 * to time/profile the firmware.
	 */
	outb(LINUX_EXIT_PORT, LINUX_START_FWCFG);
#endif

	fw_cfg_select(FW_CFG_KERNEL_ENTRY);
	kernel_entry = (void *) fw_cfg_readl_le();
	asm volatile("jmp *%2" : : "a" (0x2badb002), "b"(mb), "c"(kernel_entry));
	panic();
}

static void pvh_e820_setup()
{
	struct hvm_memmap_table_entry *pvh_e820p;
	int i, pvh_e820_sz;

	pvh_e820_sz = sizeof(struct hvm_memmap_table_entry) * e820->nr_map;

	pvh_e820p = malloc(pvh_e820_sz);
	memset(pvh_e820p, 0, pvh_e820_sz);

	for (i = 0; i < e820->nr_map; i++) {
		pvh_e820p[i].addr = e820->map[i].addr;
		pvh_e820p[i].size = e820->map[i].size;
		pvh_e820p[i].type = e820->map[i].type;
	}
	start_info.memmap_paddr = (uintptr_t)pvh_e820p;
	start_info.memmap_entries = e820->nr_map;
}

static void boot_pvh_from_fw_cfg(void)
{
	void *kernel_entry;
	uint32_t sz;
	struct linuxboot_args args;
	struct hvm_modlist_entry ramdisk_mod;

	start_info.magic = XEN_HVM_START_MAGIC_VALUE;
	start_info.version = 1;
	start_info.flags = 0;
	start_info.nr_modules = 0;
	start_info.reserved = 0;

	fw_cfg_select(FW_CFG_CMDLINE_SIZE);
	args.cmdline_size = fw_cfg_readl_le();
	args.cmdline_addr = malloc(args.cmdline_size);
	fw_cfg_read_entry(FW_CFG_CMDLINE_DATA, args.cmdline_addr,
			  args.cmdline_size);
	start_info.cmdline_paddr = (uintptr_t)args.cmdline_addr;

	fw_cfg_select(FW_CFG_INITRD_SIZE);
	args.initrd_size = fw_cfg_readl_le();
	if (args.initrd_size) {
		fw_cfg_select(FW_CFG_INITRD_ADDR);
		args.initrd_addr = (void *)fw_cfg_readl_le();

		fw_cfg_read_entry(FW_CFG_INITRD_DATA, args.initrd_addr,
			  args.initrd_size);

		ramdisk_mod.paddr = (uintptr_t)args.initrd_addr;
		ramdisk_mod.size = (uintptr_t)args.initrd_size;

		/* The first module is always ramdisk. */
		start_info.modlist_paddr = (uintptr_t)&ramdisk_mod;
		start_info.nr_modules = 1;
	}

	pvh_e820_setup();

	fw_cfg_select(FW_CFG_KERNEL_SIZE);
	sz = fw_cfg_readl_le();
	if (!sz)
		panic();

	fw_cfg_select(FW_CFG_KERNEL_ENTRY);
	kernel_entry = (void *) fw_cfg_readl_le();

#ifdef BENCHMARK_HACK
	/* Exit just before jumping to vmlinux, so that it is easy
	 * to time/profile the firmware.
	 */
	outb(LINUX_EXIT_PORT, LINUX_START_PVHBOOT);
#endif
	asm volatile("jmp *%2" : : "a" (0x2badb002),
		     "b"(&start_info), "c"(kernel_entry));
	panic();
}

void boot_from_fwcfg(void)
{
	struct linuxboot_args args;
	uint32_t kernel_size;
	enum { HEADER_PEEK_SIZE = 8192 };

	fw_cfg_select(FW_CFG_CMDLINE_SIZE);
	args.cmdline_size = fw_cfg_readl_le();
	fw_cfg_select(FW_CFG_INITRD_SIZE);
	args.initrd_size = fw_cfg_readl_le();

	/* QEMU has already split the real mode and protected mode
	 * parts.  Recombine them in args.vmlinuz_size.
	 */
	fw_cfg_select(FW_CFG_KERNEL_SIZE);
	kernel_size = fw_cfg_readl_le();
	fw_cfg_select(FW_CFG_SETUP_SIZE);
	args.vmlinuz_size = kernel_size + fw_cfg_readl_le();
	fw_cfg_select(FW_CFG_SETUP_ADDR);
	args.setup_addr = (void *)fw_cfg_readl_le();

	if (!args.vmlinuz_size)
		return;

	fw_cfg_select(FW_CFG_SETUP_DATA);
	fw_cfg_read(args.setup_addr, HEADER_PEEK_SIZE);

	if (!parse_bzimage(&args)) {
		uint8_t *header = args.setup_addr;

		if (ldl_p(header) == 0x464c457f)  /* ELF magic */
			boot_pvh_from_fw_cfg();
		boot_multiboot_from_fw_cfg();
	}

	/* SETUP_DATA already selected */
	if (args.setup_size > HEADER_PEEK_SIZE)
		fw_cfg_read(args.setup_addr + HEADER_PEEK_SIZE,
			    args.setup_size - HEADER_PEEK_SIZE);

	fw_cfg_select(FW_CFG_KERNEL_DATA);
	fw_cfg_read_entry(FW_CFG_KERNEL_DATA, args.kernel_addr, kernel_size);

	fw_cfg_read_entry(FW_CFG_CMDLINE_DATA, args.cmdline_addr, args.cmdline_size);

	if (args.initrd_size) {
		fw_cfg_read_entry(FW_CFG_INITRD_DATA, args.initrd_addr, args.initrd_size);
	}

	boot_bzimage(&args);
}
