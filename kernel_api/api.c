/* ============================================================================
 * Dionnex Kernel - Kernel Module Public API (kernel_api/api.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

void* dionnex_kernel_export_symbol(const char* name) {
    if (strcmp(name, "kmalloc") == 0) return (void*)kmalloc;
    if (strcmp(name, "kfree") == 0) return (void*)kfree;
    if (strcmp(name, "kprintf") == 0) return (void*)kprintf;
    return NULL;
}
