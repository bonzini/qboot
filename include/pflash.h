#ifndef BIOS_PFLASH_H
#define BIOS_PFLASH_H 1

#include <stdint.h>

void *pflash_base(int n, size_t *size);

#endif
