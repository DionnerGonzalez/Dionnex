/* ============================================================================
 * Dionnex Monolithic Kernel - Main Initialization Entry Point
 * ============================================================================
 */
#include <types.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/timer.h>
#include <kernel/shell.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <sched/task.h>
#include <sched/scheduler.h>
#include <fs/ramfs.h>

void kernel_main(uint32_t magic, uint32_t mbi_addr) {
    vga_init();
    printk("Dionnex Monolithic Kernel v0.1\n");

    if (magic != 0x2BADB002) {
        kernel_panic("Not Multiboot compliant magic signature!");
    }

    gdt_init();
    idt_init();
    timer_init(1000);
    keyboard_init();
    pmm_init((multiboot_info_t*)mbi_addr);
    vmm_init();
    task_init();
    scheduler_init();
    ramfs_init();

    printk("All subsystems initialized successfully\n\n");

    asm volatile("sti");

    shell_run();
}
