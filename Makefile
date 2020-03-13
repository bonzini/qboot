CC	?= $(CROSS)gcc
AS	?= $(CROSS)gcc
OBJCOPY	?= $(CROSS)objcopy
O	?= build

all: $O/bios.bin
ROMSIZE = 65536

$O: ; mkdir -p $@

CFLAGS	= \
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
	-MT $@ \
	-MMD -MP \
	-MF $O/$*.d \

ASFLAGS = $(CFLAGS)

LDFLAGS	= \
	-nostdlib \
	-m32 \
	-Wl,--build-id=none \
	-Wl,-Tflat.lds \

OBJS = \
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

$O/bios.elf: $(addprefix $O/,$(OBJS))
	$(CC) $(LDFLAGS) -o $@ $^

$O/%.bin: $O/%.elf
	$(OBJCOPY) \
		-O binary \
		-j .text \
		-j .rodata \
		-j .data \
		-j .bss \
		-j .init \
		$< \
		"$@.tmp"
	@if [ `stat -c '%s' "$@.tmp"` != $(ROMSIZE) ]; then \
		echo >&2 '$@ != $(ROMSIZE) bytes!' ; \
		exit 1 ; \
	fi
	mv "$@.tmp" "$@"

$O/%.o: %.c | $O
	$(CC) $(CFLAGS) -c -o $@ $<
$O/%.o: %.S | $O
	$(CC) $(ASFLAGS) -c -o $@ $<

clean:
	$(RM) $O/*.o $O/bios.bin $O/bios.bin.elf $O/*.d

-include $O/*.d
