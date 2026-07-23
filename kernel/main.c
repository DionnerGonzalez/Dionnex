/* ============================================================================
 * Dionnex Monolithic Kernel - Main Initialization Entry Point
 * Author: Ihosvanni Dionner Gonzalez
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
#include <drivers/apic.h>
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
#include <kernel/tss.h>
#include <kernel/process.h>
#include <kernel/scheduler_process.h>
#include <kernel/initrd.h>
#include <kernel/module_loader.h>
#include <string.h>

void kernel_main(uint32_t magic, uint32_t mbi_addr) {
    // 1. VGA Text Mode
    vga_init();

    // 2. Serial COM1 Debugging
    serial_init();

    // 3. Kernel Logger Ring Buffer
    klog_init();

    // 4. Initial KLOG message
    pr_info("Dionnex Monolithic Kernel v0.1\n");

    // 5. Multiboot verification
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kernel_panic("Invalid Multiboot magic signature!");
    }
    multiboot_info_t *mb_info = (multiboot_info_t*)mbi_addr;

    // 6. GDT Descriptor Table
    gdt_init();

    // 7. IDT Interrupt Descriptor Table
    idt_init();

    // 8. CPUID Identification & Feature Enumeration
    cpuid_init();

    // 9. Advanced Programmable Interrupt Controller (APIC)
    apic_init();

    // 10. Timer Initialization (1000 Hz)
    timer_init(1000);

    // 11. Keyboard Controller
    keyboard_init();

    // 12. Real Time Clock (RTC)
    rtc_init();

    // 13. Physical Memory Manager (PMM)
    pmm_init(mb_info);

    // 14. Virtual Memory Manager & Paging (VMM)
    vmm_init();

    // 15. Dynamic Heap Allocator (kmalloc / kfree)
    heap_init(0, DIONNEX_HEAP_SIZE_DEFAULT);

    // 16. PCI Bus Enumeration
    pci_init();

    // 17. ATA/IDE PIO Disk Driver
    ata_init();

    // 18. ACPI Power Management & Reset
    acpi_init();

    // 19. Virtual Filesystem Layer (VFS)
    vfs_init();

    // 20. RAM File System
    ramfs_init();
    vfs_mount("/ram", ramfs_get_root_node());

    // 21. Process Filesystem (/proc)
    procfs_init();
    vfs_mount("/proc", procfs_get_root_node());

    // 22. Task Management
    task_init();

    // 23. Multitasking Scheduler
    scheduler_init();

    // 24. System Calls (INT 0x80)
    syscall_init();

    // 25. Userspace Subsystems (Ring 3)
    tss_init();
    process_init();
    sched_proc_init();
    initrd_init();
    module_loader_init();

    // Additional hardware & subsystem setup
    elf_init();
    smp_init();
    vesa_init(mb_info);
    module_init_subsystem();

    // Spawn Userspace Initial Processes
    uint32_t init_sz = 0;
    uint8_t *init_data = initrd_get_file("init.elf", &init_sz);
    if (init_data && init_sz > 0) {
        process_t *init_proc = process_create("init", init_data, init_sz);
        if (init_proc) {
            sched_proc_add(init_proc);
            pr_info("Init: Started init process (pid %u)\n", init_proc->pid);
        }
    }

    uint32_t sh_sz = 0;
    uint8_t *sh_data = initrd_get_file("shell_user.elf", &sh_sz);
    if (sh_data && sh_sz > 0) {
        process_t *sh_proc = process_create("shell_user", sh_data, sh_sz);
        if (sh_proc) {
            sched_proc_add(sh_proc);
            pr_info("Init: Started user shell process (pid %u)\n", sh_proc->pid);
        }
    }

    // 26. Log completion
    pr_info("All subsystems initialized\n");

    // 26. Init commands
    klog(KLOG_INFO, "Running init commands...\n");
    ramfs_create("welcome.txt");
    ramfs_write("welcome.txt", (const uint8_t*)"Welcome to Dionnex v0.1\n", 24);

    ramfs_create("motd.txt");
    ramfs_write("motd.txt", (const uint8_t*)"Dionnex Monolithic Kernel\nGPLv2\nType 'help' for commands.\n", 57);

    printk("\n");
    printk("  Welcome to Dionnex v0.1\n");
    printk("  Type 'help' for available commands.\n");
    printk("  Type 'neofetch' for system info.\n\n");

    // Load built-in kernel modules
    printk("Module: Loading built-in modules...\n");
    uint32_t hello_sz = 0;
    uint8_t *hello_data = initrd_get_file("hello.ko", &hello_sz);
    if (hello_data && hello_sz > 0) {
        module_load_binary("hello", hello_data, hello_sz);
    }
    printk("Module: Built-in modules loaded. Use 'insmod' to load more.\n");

    // 27. Enable Hardware Interrupts
    asm volatile ("sti");

    // 28. Interactive Kernel Shell
    shell_run();
}
