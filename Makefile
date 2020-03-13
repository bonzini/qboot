CC	?= $(CROSS)gcc
AS	?= $(CROSS)gcc
OBJCOPY	?= $(CROSS)objcopy

CFLAGS	:= \
	-W \
	-Wall \
	-m32 \
	-march=i386 \
	-mregparm=3 \
	-fno-stack-protector \
	-fno-delete-null-pointer-checks \
	-ffreestanding \
	-mstringop-strategy=rep_byte \
	-minline-all-stringops \
	-fno-pic \
	-Iinclude \

ASFLAGS := $(CFLAGS)

LDFLAGS	:= \
	-nostdlib \
	-m32 \
	-Wl,--build-id=none \
	-Wl,-Tflat.lds \

all: bios.bin

%.bin: %.elf
	$(OBJCOPY) \
		-O binary \
		-j .text \
		-j .data \
		-j .rodata \
		-j .bss \
		-j .data \
		-j .init \
		$< \
		$@

OBJS := \
	code16.o \
	code32seg.o \
	cstart.o \
	entry.o \
	fw_cfg.o \
	hwsetup.o \
	linuxboot.o \
	main.o \
	malloc.o \
	mptable.o \
	pci.o \
	printf.o \
	string.o \
	smbios.o \
	tables.o \

bios.elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	$(RM) *.o bios.bin bios.bin.elf .*.d
