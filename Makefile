obj-y = code16.o entry.o main.o string.o printf.o cstart.o fw_cfg.o
obj-y += linuxboot.o malloc.o pflash.o cbfs.o tables.o hwsetup.o
obj-y += pci.o

all-y = bios.bin
all: $(all-y)

CFLAGS := -O2 -g

BIOS_CFLAGS += $(autodepend-flags) -Wall
BIOS_CFLAGS += -m32
BIOS_CFLAGS += -march=i386
BIOS_CFLAGS += -mregparm=3
BIOS_CFLAGS += -fno-stack-protector -fno-delete-null-pointer-checks
BIOS_CFLAGS += -ffreestanding
BIOS_CFLAGS += -mstringop-strategy=rep_byte -minline-all-stringops
BIOS_CFLAGS += -Iinclude

dummy := $(shell mkdir -p .deps)
autodepend-flags = -MMD -MF .deps/cc-$(patsubst %/,%,$(dir $*))-$(notdir $*).d
-include .deps/*.d

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
	rm -rf .deps
