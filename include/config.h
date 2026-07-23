/* ============================================================================
 * Dionnex Monolithic Kernel - Configuration Definitions
 * ============================================================================
 */
#ifndef _DIONNEX_CONFIG_H_
#define _DIONNEX_CONFIG_H_

#define DIONNEX_VERSION       "0.1.0"
#define DIONNEX_CODENAME      "Genesis"
#define DIONNEX_ARCH          "x86"
#define DIONNEX_ARCH_BITS     32
#define DIONNEX_PAGE_SIZE     4096
#define DIONNEX_MAX_TASKS     256
#define DIONNEX_MAX_FILES     256
#define DIONNEX_HEAP_SIZE     (4 * 1024 * 1024) // 4MB initial heap
#define DIONNEX_MAX_MODULES   64
#define DIONNEX_MAX_PCI_DEVS  256
#define DIONNEX_MAX_SYSCALLS  256
#define DIONNEX_KLOG_SIZE     4096
#define DIONNEX_SHELL_HIST    20
#define DIONNEX_SHELL_CMD_LEN 256

#define __DIONNEX_BUILD_DATE__ __DATE__
#define __DIONNEX_BUILD_TIME__ __TIME__

#endif /* _DIONNEX_CONFIG_H_ */
