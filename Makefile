obj16-y = e820.o int10.o int15.o
obj-y = $(obj16-y) entry.o main.o string.o printf.o cstart.o fw_cfg.o
obj-y += linuxboot.o

all-y = bios.bin

CFLAGS := -O2 -Wall -g

BIOS_CFLAGS += -m32
BIOS_CFLAGS += -march=i386
BIOS_CFLAGS += -mregparm=3
BIOS_CFLAGS += -fno-stack-protector -fno-delete-null-pointer-checks
BIOS_CFLAGS += -ffreestanding
BIOS_CFLAGS += -Iinclude
$(obj16-y): BIOS_CFLAGS += -include code16gcc.h

all: $(all-y)

.PRECIOUS: %.o
%.o: %.c
	$(CC) $(CFLAGS) $(BIOS_CFLAGS) -c -s $< -o $@
%.o: %.S
	$(CC) $(CFLAGS) $(BIOS_CFLAGS) -c -s $< -o $@

bios.bin.elf: $(obj-y) flat.lds
	$(LD) -T flat.lds -o bios.bin.elf $(obj-y)

bios.bin: bios.bin.elf
	objcopy -O binary bios.bin.elf bios.bin

clean:
	rm -f $(obj-y) $(all-y) bios.bin.elf
