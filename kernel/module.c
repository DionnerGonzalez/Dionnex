/* ============================================================================
 * Dionnex Monolithic Kernel - Kernel Module Loader Implementation
 * ============================================================================
 */
#include <kernel/module.h>
#include <kernel/printk.h>
#include <mm/heap.h>
#include <string.h>

static module_t *module_list_head = NULL;
static uint32_t module_count = 0;

void module_init_subsystem(void) {
    module_list_head = NULL;
    module_count = 0;
    printk("Module: Subsystem initialized\n");
}

int module_register(module_t *mod) {
    if (!mod || !mod->name[0]) return -1;

    if (module_find(mod->name) != NULL) {
        printk("Module: Module '%s' is already registered\n", mod->name);
        return -1;
    }

    if (mod->init) {
        int res = mod->init();
        if (res != 0) {
            printk("Module: Initialization of '%s' failed with error %d\n", mod->name, res);
            return res;
        }
    }

    mod->state = MODULE_STATE_LOADED;
    mod->next = module_list_head;
    module_list_head = mod;
    module_count++;

    printk("Module: Loaded '%s' v%s by %s\n", mod->name, mod->version, mod->author);
    return 0;
}

int module_unregister(const char *name) {
    if (!name) return -1;

    module_t *curr = module_list_head;
    module_t *prev = NULL;

    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            if (curr->cleanup) {
                curr->cleanup();
            }
            curr->state = MODULE_STATE_UNLOADED;

            if (prev) {
                prev->next = curr->next;
            } else {
                module_list_head = curr->next;
            }

            module_count--;
            printk("Module: Unloaded '%s'\n", name);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }

    printk("Module: Module '%s' not found\n", name);
    return -1;
}

module_t *module_find(const char *name) {
    if (!name) return NULL;
    module_t *curr = module_list_head;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void module_list(void) {
    printk("NAME                 VERSION    AUTHOR               STATE\n");
    printk("----                 -------    ------               -----\n");

    module_t *curr = module_list_head;
    while (curr) {
        const char *state_str = (curr->state == MODULE_STATE_LOADED) ? "LOADED" : "UNLOADED";
        printk("%-20s %-10s %-20s %s\n", curr->name, curr->version, curr->author, state_str);
        curr = curr->next;
    }

    if (!module_list_head) {
        printk("(no modules loaded)\n");
    }
}

int module_load_from_memory(const uint8_t *data, uint32_t size) {
    if (!data || size < sizeof(module_t)) return -1;

    module_t *mod = (module_t*)kmalloc(sizeof(module_t));
    if (!mod) return -1;

    memcpy(mod, data, sizeof(module_t));
    return module_register(mod);
}
