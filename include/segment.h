#ifndef BIOS_SEGMENT_H
#define BIOS_SEGMENT_H

static inline uint32_t segment_to_flat(uint16_t selector, uint16_t offset)
{
	return ((uint32_t)selector << 4) + (uint32_t) offset;
}

static inline uint16_t flat_to_seg16(uint32_t address)
{
	return (address >> 4) & 0xf000;
}

static inline uint16_t flat_to_off16(uint32_t address)
{
	return address & 65535;
}

#endif /* KVM_SEGMENT_H */
