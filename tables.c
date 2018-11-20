#include "bios.h"
#include "stdio.h"
#include "fw_cfg.h"
#include "string.h"
#include "start_info.h"

extern struct hvm_start_info start_info;

struct loader_cmd {
	uint32_t cmd;
	union {
#define CMD_QUIT 0
#define CMD_ALLOC 1
		struct {
			char file[56];
			uint32_t align;
			uint8_t zone;
		} alloc;
#define CMD_PTR 2
		struct {
			char dest[56];
			char src[56];
			uint32_t offset;
			uint8_t size;
		} ptr;
#define CMD_CHECKSUM 3
		struct {
			char file[56];
			uint32_t offset;
			uint32_t start;
			uint32_t len;
		} checksum;
		uint8_t pad[124];
	};
} __attribute__((__packed__));

enum {
	ALLOC_HIGH = 1,
	ALLOC_FSEG = 2
};

static uint8_t *file_address[20];

static inline void *id_to_addr(int fw_cfg_id)
{
	return file_address[fw_cfg_id];
}

static inline void set_file_addr(int fw_cfg_id, void *p)
{
	file_address[fw_cfg_id] = p;
}

static void do_alloc(char *file, uint32_t align, uint8_t zone)
{
	int id = fw_cfg_file_id(file);
	int n = fw_cfg_file_size(id);
	char *p;

	if (id == -1)
		panic();

	if (align < 16)
		align = 16;

	if (zone == ALLOC_FSEG)
		p = malloc_fseg_align(n, align);
	else
		p = malloc_align(n, align);

	set_file_addr(id, p);
	fw_cfg_read_file(id, p, n);

	/* For PVH boot, save the PA where the RSDP is stored */
	if (zone == ALLOC_FSEG) {
		if (!memcmp(p, "RSD PTR ", 8)) {
			start_info.rsdp_paddr = (uintptr_t)id_to_addr(id);
		}
	}
}

static void do_ptr(char *dest, char *src, uint32_t offset, uint8_t size)
{
	char *p, *q;
	int id;
	union {
		long long ll;
		char b[8];
	} data;

	id = fw_cfg_file_id(src);
	p = id_to_addr(id);
	if (!p)
		panic();

	id = fw_cfg_file_id(dest);
	q = id_to_addr(id);
	if (!q)
		panic();

	q += offset;

	/* Assumes little endian */
	data.ll = 0;
	memcpy(&data.b, q, size);
	data.ll += (uintptr_t) p;
	memcpy(q, &data.b, size);
}

static inline uint8_t csum8(uint8_t *buf, uint32_t len)
{
	uint32_t s = 0;
	while (len-- > 0)
		s += *buf++;
	return s;
}
static void do_checksum(char *file, uint32_t offset, uint32_t start, uint32_t len)
{
	uint8_t *p;
	int id;
	int n;

	id = fw_cfg_file_id(file);
	p = id_to_addr(id);
	if (!p)
		panic();

	n = fw_cfg_file_size(id);
	if (offset >= n || n < start || len > n - start)
		panic();

	p[offset] -= csum8(&p[start], len);
}

void extract_acpi(void)
{
	int id = fw_cfg_file_id("etc/table-loader");
	int n = fw_cfg_file_size(id);
	struct loader_cmd script[n / sizeof(struct loader_cmd)];
	int i;

	if (!n)
		return;

	fw_cfg_read_file(id, script, n);

	for (i = 0; i < ARRAY_SIZE(script); i++) {
		struct loader_cmd *s = &script[i];
		switch(script[i].cmd) {
		case CMD_ALLOC:
			do_alloc(s->alloc.file, s->alloc.align, s->alloc.zone);
			break;
		case CMD_PTR:
			do_ptr(s->ptr.dest, s->ptr.src, s->ptr.offset, s->ptr.size);
			break;
		case CMD_CHECKSUM:
			do_checksum(s->checksum.file, s->checksum.offset,
				    s->checksum.start, s->checksum.len);
			break;
		case CMD_QUIT:
			return;
		default:
			panic();
		}
	}
}
