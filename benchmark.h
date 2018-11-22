#ifndef BENCHMARK_H
#define BENCHMARK_H

/* IO ports for different exit points */
#define LINUX_EXIT_PORT 0xf4
#define FW_EXIT_PORT    0xf5

/* Exit point values */
#define FW_START    1
#define LINUX_START_FWCFG 2
#define LINUX_START_BOOT  3
#define LINUX_START_PVHBOOT  4

#endif /* BENCHMARK_H */
