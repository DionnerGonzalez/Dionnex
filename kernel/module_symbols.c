/* ============================================================================
 * Dionnex Monolithic Kernel - Symbol Export Table
 * Author: Ihosvanni Dionner Gonzalez
 * Adapted from Linux kernel: kernel/module/kallsyms.c (GPLv2)
 * ============================================================================
 */

#include <kernel/module_symbols.h>
#include <kernel/module_loader.h>
#include <kernel/printk.h>
#include <kernel/klog.h>
#include <kernel/panic.h>
#include <kernel/cpuid.h>
#include <kernel/timer.h>
#include <kernel/process.h>
#include <drivers/vga.h>
#include <drivers/serial.h>
#include <drivers/pci.h>
#include <drivers/apic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <fs/vfs.h>
#include <string.h>

#define EXPORT_SYM(fn) { #fn, (uint32_t)&fn }

const kernel_symbol_t kernel_symbols[] = {
    // Printk / logging
    EXPORT_SYM(printk),
    EXPORT_SYM(klog),
    EXPORT_SYM(klog_dump),

    // Memory
    EXPORT_SYM(kmalloc),
    EXPORT_SYM(kfree),
    EXPORT_SYM(krealloc),
    EXPORT_SYM(kcalloc),
    EXPORT_SYM(pmm_alloc_page),
    EXPORT_SYM(pmm_free_page),
    EXPORT_SYM(pmm_get_free_count),
    EXPORT_SYM(pmm_get_total_count),
    EXPORT_SYM(vmm_map_page),
    EXPORT_SYM(vmm_unmap_page),
    EXPORT_SYM(vmm_get_phys),

    // String
    EXPORT_SYM(memset),
    EXPORT_SYM(memcpy),
    EXPORT_SYM(memmove),
    EXPORT_SYM(memcmp),
    EXPORT_SYM(strlen),
    EXPORT_SYM(strcpy),
    EXPORT_SYM(strncpy),
    EXPORT_SYM(strcmp),
    EXPORT_SYM(strncmp),
    EXPORT_SYM(strcat),
    EXPORT_SYM(strchr),
    EXPORT_SYM(strstr),
    EXPORT_SYM(itoa),
    EXPORT_SYM(utoa),
    EXPORT_SYM(atoi),

    // VGA
    EXPORT_SYM(vga_clear),
    EXPORT_SYM(vga_putchar),
    EXPORT_SYM(vga_write),
    EXPORT_SYM(vga_set_color),
    EXPORT_SYM(vga_write_hex),
    EXPORT_SYM(vga_write_dec),

    // Serial
    EXPORT_SYM(serial_write),
    EXPORT_SYM(serial_putchar),

    // VFS
    EXPORT_SYM(vfs_open),
    EXPORT_SYM(vfs_read),
    EXPORT_SYM(vfs_write),
    EXPORT_SYM(vfs_list),
    EXPORT_SYM(vfs_mount),

    // Timer
    EXPORT_SYM(timer_get_ticks),
    EXPORT_SYM(timer_sleep),

    // Process
    EXPORT_SYM(process_get_current),
    EXPORT_SYM(process_get_by_pid),
    EXPORT_SYM(process_get_count),

    // PCI
    EXPORT_SYM(pci_get_device),
    EXPORT_SYM(pci_get_class),
    EXPORT_SYM(pci_get_count),
    EXPORT_SYM(pci_config_read32),
    EXPORT_SYM(pci_config_write32),

    // CPUID
    EXPORT_SYM(cpuid_print_info),

    // APIC
    EXPORT_SYM(apic_eoi),

    // Module system
    EXPORT_SYM(module_load_binary),
    EXPORT_SYM(module_unload_binary),
    EXPORT_SYM(module_list_loaded),

    // Panic
    EXPORT_SYM(kernel_panic)
};

const uint32_t kernel_symbol_count = sizeof(kernel_symbols) / sizeof(kernel_symbol_t);

int module_resolve_symbol(const char *name, uint32_t *addr) {
    if (!name || !addr) return -1;
    for (uint32_t i = 0; i < kernel_symbol_count; i++) {
        if (strcmp(name, kernel_symbols[i].name) == 0) {
            *addr = kernel_symbols[i].addr;
            return 0;
        }
    }
    return -1;
}
