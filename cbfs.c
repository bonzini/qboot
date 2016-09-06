#include "bios.h"
#include "stdio.h"
#include "ioport.h"
#include "string.h"
#include "bswap.h"
#include "linuxboot.h"

#define CBFS_HEADER_MAGIC    0x4F524243		// ORBC
#define CBFS_HEADER_VERSION1 0x31313131
#define CBFS_HEADER_VERSION2 0x31313132

static const char file_magic[8] = "LARCHIVE";

struct cbfs_header {
    uint32_t magic;
    uint32_t version;
    uint32_t romsize;
    uint32_t bootblocksize;
    uint32_t align;
    uint32_t offset;
    uint32_t pad[2];
} __attribute__((__packed__));

struct cbfs_file_header {
    char magic[8];
    uint32_t len;
    uint32_t type;
    uint32_t checksum;
    uint32_t offset;
} __attribute__((__packed__));


struct cbfs_file {
	uint32_t size;
	const char *buf;
	char name[57];
	struct cbfs_file *next;
};

static struct cbfs_file *files;

static bool cbfs_setup(const char *base, size_t sz)
{
	uint32_t ofs;
	struct cbfs_header hdr;
	struct cbfs_file **pnext;
	uint32_t align;
       
	ofs = sz + (intptr_t)(int32_t)ldl_le_p(base + sz - 4);
	if (ofs >= sz - sizeof(struct cbfs_header) ||
	    ldl_be_p(base + ofs) != CBFS_HEADER_MAGIC)
		ofs = 0;

	for (; ofs + sizeof(struct cbfs_header) < sz; ofs++) {
		if (ldl_be_p(base + ofs) != CBFS_HEADER_MAGIC)
			continue;
		memcpy(&hdr, base + ofs, sizeof(hdr));
		if (ldl_be_p(&hdr.version) != CBFS_HEADER_VERSION1 &&
		    ldl_be_p(&hdr.version) != CBFS_HEADER_VERSION2)
			continue;
		break;
	}

	if (ofs + sizeof(struct cbfs_header) >= sz)
		return false;

	pnext = &files;
	ofs = ldl_be_p(&hdr.offset);
	align = ldl_be_p(&hdr.align);
	while (ofs + sizeof(struct cbfs_file_header) < sz) {
		struct cbfs_file_header fhdr;
		struct cbfs_file *f;

		memcpy(&fhdr, base + ofs, sizeof(fhdr));
		if (memcmp(&fhdr.magic, file_magic, sizeof(file_magic)))
			break;

		f = malloc_fseg(sizeof(*f));
		*pnext = f;

		f->size = ldl_be_p(&fhdr.len);
		f->buf = base + ofs + ldl_be_p(&fhdr.offset);
		strcpy(f->name, base + ofs + sizeof(fhdr));
		f->next = NULL;
		pnext = &f->next;

		ofs = f->buf + f->size - base;
		ofs = (ofs + align - 1) & -align;
	}
	return files != NULL;
}

static struct cbfs_file *cbfs_file_find(const char *name)
{
	struct cbfs_file *f = files;

	for (f = files; f; f = f->next)
		if (!strcmp(name, f->name))
			return f;

	return NULL;
}

uint32_t cbfs_size(const char *name)
{
	struct cbfs_file *f = cbfs_file_find(name);
	if (!f)
		return 0;

	return f->size;
}

void cbfs_read(const char *name, void *buf, size_t size, size_t skip)
{
	struct cbfs_file *f = cbfs_file_find(name);
	if (!f)
		panic();

	if (skip > f->size)
		return;
	if (size + skip > f->size)
		size = f->size - skip;
	memcpy(buf, f->buf + skip, size);
}

bool cbfs_boot(void)
{
	struct linuxboot_args args;

	args.vmlinuz_size = cbfs_size("vmlinuz");
	if (!args.vmlinuz_size)
		return false;

	args.initrd_size = cbfs_size("initrd");
	args.cmdline_size = cbfs_size("cmdline");

	cbfs_read("vmlinuz", args.header, sizeof(args.header), 0);
	if (!parse_bzimage(&args))
		return false;

	cbfs_read("vmlinuz", args.setup_addr, args.setup_size, 0);
	cbfs_read("vmlinuz", args.kernel_addr, args.kernel_size, args.setup_size);

	if (args.initrd_size)
		cbfs_read("initrd", args.initrd_addr, args.initrd_size, 0);
	if (args.cmdline_size)
		cbfs_read("cmdline", args.cmdline_addr, args.cmdline_size, 0);

	boot_bzimage(&args);
	return true;
}

bool boot_from_cbfs(void *base, size_t sz)
{
	return cbfs_setup(base, sz) && cbfs_boot();
}

