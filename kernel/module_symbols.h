/* ============================================================================
 * Dionnex Monolithic Kernel - Symbol Export Table Header
 * Author: Ihosvanni Dionner Gonzalez
 * Adapted from Linux kernel (GPLv2)
 * ============================================================================
 */
#ifndef _DIONNEX_MODULE_SYMBOLS_H_
#define _DIONNEX_MODULE_SYMBOLS_H_

#include <types.h>

struct kernel_symbol_struct {
    const char *name;
    uint32_t addr;
};

typedef struct kernel_symbol_struct kernel_symbol_t;

extern const kernel_symbol_t kernel_symbols[];
extern const uint32_t kernel_symbol_count;

int module_resolve_symbol(const char *name, uint32_t *addr);

#endif /* _DIONNEX_MODULE_SYMBOLS_H_ */
