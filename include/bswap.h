#ifndef BSWAP_H
#define BSWAP_H 1

static inline uint16_t bswap16(uint16_t x)
{
	return __builtin_bswap16(x);
}

static inline uint32_t bswap32(uint32_t x)
{
	return __builtin_bswap32(x);
}

static inline uint64_t bswap64(uint64_t x)
{
	return __builtin_bswap64(x);
}

static inline uint32_t ldl_le_p(const void *p)
{
	uint32_t val;
	memcpy(&val, p, 4);
	return val;
}

static inline uint32_t ldl_be_p(const void *p)
{
	uint32_t val;
	memcpy(&val, p, 4);
	return bswap32(val);
}


#endif
