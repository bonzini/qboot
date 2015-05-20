obj16-y = e820.o int10.o int15.o
obj-y = $(obj16-y) entry.o
all-y = bios.bin

BIOS_CFLAGS += -m32
BIOS_CFLAGS += -march=i386
BIOS_CFLAGS += -mregparm=3
BIOS_CFLAGS += -fno-stack-protector
BIOS_CFLAGS += -Iinclude
$(obj16-y): BIOS_CFLAGS += -include code16gcc.h

all: $(all-y)

.PRECIOUS: %.o
%.o: %.c
	$(CC) $(CFLAGS) $(BIOS_CFLAGS) -c -s $< -o $@
%.o: %.S
	$(CC) $(CFLAGS) $(BIOS_CFLAGS) -c -s $< -o $@

bios.bin.elf: $(obj-y) rom.ld.S
	$(LD) -T rom.ld.S -o bios.bin.elf $(obj-y)

bios.bin: bios.bin.elf
	objcopy -O binary bios.bin.elf bios.bin

clean:
	rm $(obj-y) $(all-y) bios.bin.elf
