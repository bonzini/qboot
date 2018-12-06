#ifndef MEMACCESS_H_
#define MEMACCESS_H_

#include "string.h"

static inline uint16_t lduw_p(void *p)
{
	uint16_t val;
	memcpy(&val, p, 2);
	return val;
}

static inline uint32_t ldl_p(void *p)
{
	uint32_t val;
	memcpy(&val, p, 4);
	return val;
}

static inline void stw_p(void *p, uint16_t val)
{
	memcpy(p, &val, 2);
}

static inline void stl_p(void *p, uint32_t val)
{
	memcpy(p, &val, 4);
}

#endif /* MEMACCESS_H_ */
