/*
 * Dionnex Module ABI Header
 */

#ifndef _DIONNEX_MODULE_ABI_H
#define _DIONNEX_MODULE_ABI_H

#include <ABI/kernel/internal/kabi.h>

#define MODULE_NAME_LEN 64

struct module_symbol {
	const char *name;
	virt_addr_t addr;
};

struct module {
	char name[MODULE_NAME_LEN];
	int state;
	virt_addr_t base_addr;
	size_t core_size;
	int (*init)(void);
	void (*cleanup)(void);
};

#define module_init(fn) int init_module(void) { return fn(); }
#define module_exit(fn) void cleanup_module(void) { fn(); }

#endif /* _DIONOS_MODULE_ABI_H */
