#include "bios.h"
#include "ioport.h"
#include "string.h"
#include "fw_cfg.h"

struct fw_cfg_file {
	uint32_t size;
	uint16_t select;
	char name[57];
};

static struct fw_cfg_file files[32];

void fw_cfg_setup(void)
{
	int i, n;

	fw_cfg_select(FW_CFG_FILE_DIR);
	n = fw_cfg_readl_be();
	if (n > ARRAY_SIZE(files))
		n = ARRAY_SIZE(files);

	for (i = 0; i < n; i++) {
		files[i].size = fw_cfg_readl_be();
		files[i].select = fw_cfg_readw_be();
		fw_cfg_readw_be();
		fw_cfg_read(files[i].name, sizeof(files[i].name) - 1);
	}
}

int fw_cfg_file_id(char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(files); i++)
		if (!strcmp(name, files[i].name))
			return i;

	return -1;
}

uint32_t fw_cfg_file_size(int id)
{
	return files[id].size;
}

void fw_cfg_file_select(int id)
{
	fw_cfg_select(files[id].select);
}
