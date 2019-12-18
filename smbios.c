#include <stdint.h>
#include "smbios.h"
#include "string.h"
#include "fw_cfg.h"

#define VERSION "0.1"
#define BIOS_NAME "qboot"
#define BIOS_DATE "11/11/2019"

struct smbios_entry_point {
    uint32_t signature;
    uint8_t checksum;
    uint8_t length;
    uint8_t smbios_major_version;
    uint8_t smbios_minor_version;
    uint16_t max_structure_size;
    uint8_t entry_point_revision;
    uint8_t formatted_area[5];
    char intermediate_anchor_string[5];
    uint8_t intermediate_checksum;
    uint16_t structure_table_length;
    uint32_t structure_table_address;
    uint16_t number_of_structures;
    uint8_t smbios_bcd_revision;
} __attribute__((packed));

struct smbios_structure_header {
    uint8_t type;
    uint8_t length;
    uint16_t handle;
} __attribute__((packed));

struct smbios_type_0 {
    struct smbios_structure_header header;
    uint8_t vendor_str;
    uint8_t bios_version_str;
    uint16_t bios_starting_address_segment;
    uint8_t bios_release_date_str;
    uint8_t bios_rom_size;
    uint8_t bios_characteristics[8];
    uint8_t bios_characteristics_extension_bytes[2];
    uint8_t system_bios_major_release;
    uint8_t system_bios_minor_release;
    uint8_t embedded_controller_major_release;
    uint8_t embedded_controller_minor_release;
} __attribute__((packed));

#define set_str_field_or_skip(type, field, value)                       \
    do {                                                                \
        int size = (value != NULL) ? strlen(value) + 1 : 0;             \
        if (size > 1) {                                                 \
            memcpy(end, value, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } else {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } while (0)

static void smbios_new_type_0(void *start, const char *vendor,
                        const char *version, const char *date)
{
    struct smbios_type_0 *p = (struct smbios_type_0 *)start;
    char *end = (char *)start + sizeof(struct smbios_type_0);
    int str_index = 0;

    p->header.type = 0;
    p->header.length = sizeof(struct smbios_type_0);
    p->header.handle = 0;

    set_str_field_or_skip(0, vendor_str, vendor);
    set_str_field_or_skip(0, bios_version_str, version);
    p->bios_starting_address_segment = 0xe800;
    set_str_field_or_skip(0, bios_release_date_str, date);

    p->bios_rom_size = 0; /* FIXME */

    /* BIOS characteristics not supported */
    memset(p->bios_characteristics, 0, 8);
    p->bios_characteristics[0] = 0x08;

    /* Enable targeted content distribution (needed for SVVP) */
    p->bios_characteristics_extension_bytes[0] = 0;
    p->bios_characteristics_extension_bytes[1] = 4;

    p->system_bios_major_release = 0;
    p->system_bios_minor_release = 0;
    p->embedded_controller_major_release = 0xFF;
    p->embedded_controller_minor_release = 0xFF;

    *end = 0;
    end++;
    if (!str_index) {
        *end = 0;
        end++;
    }
}

static struct smbios_structure_header *smbios_next(struct smbios_entry_point *ep,
                                                   struct smbios_structure_header *hdr)
{
    if (!ep)
        return NULL;
    void *start = (void *)ep->structure_table_address;
    void *end = start + ep->structure_table_length;
    void *ptr;

    if (hdr == NULL)
        ptr = start;
    else {
        ptr = hdr;
        if (ptr + sizeof(*hdr) > end)
            return NULL;
        ptr += hdr->length + 2;
        while (ptr < end &&
               (*(uint8_t*)(ptr-1) != '\0' || *(uint8_t*)(ptr-2) != '\0'))
            ptr++;
    }
    hdr = ptr;
    if (ptr >= end || ptr + sizeof(*hdr) >= end || ptr + hdr->length >= end)
        return NULL;
    return hdr;
}

void extract_smbios(void)
{
    int id;
    struct smbios_entry_point *ep;
    uint8_t *qtables;
    uint16_t qtables_length;
    struct smbios_structure_header *table_header = NULL;
    int need_t0 = 1;
    uint16_t t0_len = sizeof(struct smbios_type_0) + strlen(BIOS_NAME) +
                        strlen(VERSION) + strlen(BIOS_DATE) + 4;

    id = fw_cfg_file_id("etc/smbios/smbios-anchor");
    if (id == -1 || (sizeof(*ep) != fw_cfg_file_size(id)))
        return ;
    /* malloc_fseg is 16-byte alignment default */
    if (!(ep = malloc_fseg(sizeof(*ep))))
        return;
    fw_cfg_read_file(id, ep, sizeof(*ep));

    qtables_length = ep->structure_table_length;
    id = fw_cfg_file_id("etc/smbios/smbios-tables");
    if (id == -1 || qtables_length != fw_cfg_file_size(id))
        return ;
    if (!(qtables = malloc_fseg(qtables_length + t0_len)))
        return ;
    qtables += t0_len;
    fw_cfg_read_file(id, qtables, qtables_length);

    ep->structure_table_address = (uint32_t) qtables;
    ep->structure_table_length = qtables_length;

    while ((table_header = smbios_next(ep, table_header))) {
         if (table_header->type == 0) {
            need_t0 = 0;
            break;
         }
    }

    if (need_t0) {
        smbios_new_type_0(qtables - t0_len, BIOS_NAME, VERSION, BIOS_DATE);
        ep->number_of_structures++;
        ep->structure_table_address -= t0_len;
        ep->structure_table_length += t0_len;
        if (t0_len > ep->max_structure_size)
            ep->max_structure_size = t0_len;
    }

    ep->checksum -= csum8((void *) ep, 0x10);
    ep->intermediate_checksum -= csum8((void *) ep + 0x10, ep->length - 0x10);
}
