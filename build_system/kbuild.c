/* ============================================================================
 * Dionnex Kernel - Kernel Module Build Engine (build_system/kbuild.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

void kbuild_process_module(const char* module_name) {
    kprintf("[KBUILD] Compilando e integrando módulo: %s\n", module_name);
}
