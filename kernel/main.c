/* ============================================================================
 * Dionnex Monolithic Kernel - Main Initialization Entry Point
 * ============================================================================
 */
#include <types.h>
#include <config.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/timer.h>
#include <kernel/klog.h>
#include <kernel/cpuid.h>
#include <kernel/syscall.h>
#include <kernel/elf.h>
#include <kernel/module.h>
#include <kernel/debug.h>
#include <kernel/smp.h>
#include <kernel/shell.h>
#include <kernel/multiboot.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <drivers/rtc.h>
#include <drivers/pci.h>
#include <drivers/ata.h>
#include <drivers/acpi.h>
#include <drivers/vesa.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <sched/task.h>
#include <sched/scheduler.h>
#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/procfs.h>

void kernel_main(uint32_t magic, uint32_t mbi_addr) {
    // 1. VGA Text Mode
    vga_init();

    // 2. Serial COM1 Debugging
    serial_init();

    // 3. Kernel Logger Ring Buffer
    klog_init();

    // 4. Initial KLOG message
    klog(KLOG_INFO, "Dionnex Monolithic Kernel v0.1.0 starting...\n");

    // Multiboot verification
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kernel_panic("Invalid Multiboot magic signature!");
    }
    multiboot_info_t *mb_info = (multiboot_info_t*)mbi_addr;

    // 5. GDT Descriptor Table
    gdt_init();

    // 6. IDT Interrupt Descriptor Table
    idt_init();

    // 7. Programmable Interrupt Controller (PIC)
    pic_init();

    // 8. Programmable Interval Timer (PIT @ 100 Hz)
    timer_init(100);

    // 9. Keyboard Controller
    keyboard_init();

    // 10. CPUID Identification & Feature Enumeration
    cpuid_print_info();

    // 11. Physical Memory Manager (PMM)
    pmm_init(mb_info);

    // 12. Virtual Memory Manager & Paging (VMM)
    vmm_init();

    // 13. Dynamic Heap Allocator (kmalloc / kfree)
    heap_init(0, DIONNEX_HEAP_SIZE_DEFAULT);

    // 14. Real Time Clock (RTC)
    rtc_init();

    // 15. PCI Bus Enumeration
    pci_init();

    // 16. ATA/IDE PIO Disk Driver
    ata_init();

    // 17. Virtual Filesystem Layer (VFS)
    vfs_init();

    // 18. RAM File System
    ramfs_init();

    // 19. Mount RAMFS to /ram
    vfs_mount("/ram", ramfs_get_root_node());

    // 20. Process Filesystem (/proc)
    procfs_init();

    // 21. Mount ProcFS to /proc
    vfs_mount("/proc", procfs_get_root_node());

    // 22. System Calls (INT 0x80)
    syscall_init();

    // 23. ELF32 Executable Loader
    elf_init();

    // 24. Symmetric Multiprocessing Detection
    smp_init();

    // 25. ACPI Power Management & Reset
    acpi_init();

    // 26. VESA VBE Graphics Framebuffer
    vesa_init(mb_info);

    // 27. Dynamic Kernel Module Subsystem
    module_init_subsystem();

    // 28. Task Management
    task_init();

    // 29. Multitasking Scheduler
    scheduler_init();

    // Enable Hardware Interrupts
    asm volatile ("sti");

    klog(KLOG_INFO, "All kernel subsystems successfully initialized\n");

    // 30 & 31. Interactive Kernel Shell
    shell_run();
}
