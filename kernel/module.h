/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Module Loader Header
 * ============================================================================
 */
#ifndef _DIONNEX_MODULE_H_
#define _DIONNEX_MODULE_H_

#include <types.h>

#define MODULE_STATE_UNLOADED 0
#define MODULE_STATE_LOADED   1

struct module {
    char name[64];
    char author[64];
    char version[16];
    int (*init)(void);
    void (*cleanup)(void);
    uint8_t state;
    struct module *next;
};

typedef struct module module_t;

void module_init_subsystem(void);
int module_register(module_t *mod);
int module_unregister(const char *name);
module_t *module_find(const char *name);
void module_list(void);
int module_load_from_memory(const uint8_t *data, uint32_t size);

#endif /* _DIONNEX_MODULE_H_ */
