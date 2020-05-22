project('qboot', 'c', meson_version: '>=0.49.0')

cc = meson.get_compiler('c')
objcopy = find_program('objcopy')

c_args = [
  '-m32',
  '-march=i386',
  '-mregparm=3',
  '-fno-stack-protector',
  '-fno-delete-null-pointer-checks',
  '-ffreestanding',
  '-mstringop-strategy=rep_byte',
  '-minline-all-stringops',
  '-fno-pic',
]

link_args = ['-nostdlib', '-m32']
link_args += cc.get_supported_link_arguments('-Wl,--build-id=none')
link_args += '-Wl,-T' + meson.current_source_dir() / 'flat.lds'
link_args += cc.get_supported_link_arguments(['-no-pie'])

elf = executable(
  'bios.bin.elf',
  files(
    'code16.c',
    'code32seg.c',
    'cstart.S',
    'entry.S',
    'fw_cfg.c',
    'hwsetup.c',
    'linuxboot.c',
    'main.c',
    'malloc.c',
    'mptable.c',
    'pci.c',
    'printf.c',
    'string.c',
    'smbios.c',
    'tables.c',
  ),
  c_args: c_args,
  include_directories: include_directories('include'),
  link_args: link_args,
)

bin = custom_target(
  'bios.bin',
  output: 'bios.bin',
  input: elf,
  command: [objcopy, '-O', 'binary', '@INPUT@', '@OUTPUT@'],
  build_by_default: true,
)
