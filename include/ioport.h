#ifndef BIOS_IOPORT_H
#define BIOS_IOPORT_H 1

static inline void outsb(unsigned short port, void *buf, int len)
{
	asm volatile("rep outsb %%ds:(%0), %3" : "=S" (buf), "=c" (len) : "m"(buf), "Nd"(port), "0" (buf), "1" (len));
}

static inline void insb(void *buf, unsigned short port, int len)
{
	asm volatile("rep insb %3, %%es:(%0)" : "=D" (buf), "=c" (len), "=m"(buf) : "Nd"(port), "0" (buf), "1" (len));
}

static inline unsigned char inb(unsigned short port)
{
	unsigned char val;
	asm volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
	return val;
}

static inline unsigned short inw(unsigned short port)
{
	unsigned short val;
	asm volatile("inw %1, %0" : "=a"(val) : "Nd"(port));
	return val;
}

static inline unsigned inl(unsigned short port)
{
	unsigned val;
	asm volatile("inl %1, %0" : "=a"(val) : "Nd"(port));
	return val;
}

static inline void outb(unsigned short port, unsigned char val)
{
	asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw(unsigned short port, unsigned short val)
{
	asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outl(unsigned short port, unsigned val)
{
	asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

#endif
