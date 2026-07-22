/* ============================================================================
 * Dionnex Kernel - Core Subsystem Initializer (kernel/init.c)
 * ============================================================================
 */
#include <kernel/kernel.h>
#include <kernel/kabi.h>

void start_kernel(void) {
    vga_clear();
    vga_set_color(0x0B, 0x00); // Light cyan on black
    kprintf("========================================================\n");
    kprintf("   Dionnex Modern Microkernel Subsystem Architecture    \n");
    kprintf("========================================================\n\n");

    /* 1. Architecture & CPU Control */
    kprintf("[INIT] Setting up Global Descriptor Table (GDT)...\n");
    gdt_init();

    kprintf("[INIT] Setting up Interrupt Descriptor Table (IDT)...\n");
    idt_init();

    kprintf("[INIT] Remapping 8259 Programmable Interrupt Controller (PIC)...\n");
    pic_remap();

    kprintf("[INIT] Initializing 8253 Programmable Interval Timer (PIT @ 100Hz)...\n");
    pit_init(100);

    kprintf("[INIT] Configuring Task State Segment (TSS)...\n");
    tss_init();

    /* 2. Memory Subsystem */
    kprintf("[INIT] Initializing Physical Memory Manager (PMM 128MB)...\n");
    pmm_init(128);

    kprintf("[INIT] Initializing Virtual Memory Manager (VMM Page Directory)...\n");
    vmm_init();

    kprintf("[INIT] Initializing Kernel Dynamic Heap Allocator (kmalloc)...\n");
    kmalloc_init();

    /* 3. Drivers & Hardware */
    kprintf("[INIT] Hardware Abstraction Layer (HAL)...\n");
    hal_init();

    kprintf("[INIT] Power Management (ACPI/PM)...\n");
    power_management_init();

    kprintf("[INIT] Driver Framework & Device Tree...\n");
    driver_framework_init();

    kprintf("[INIT] Registering System Interrupt Requests (IRQs)...\n");
    irq_init();

    kprintf("[INIT] Initializing PS/2 Keyboard Driver...\n");
    keyboard_init();

    /* 4. Filesystem, Networking & Security */
    kprintf("[INIT] Initializing Virtual File System (VFS)...\n");
    vfs_init();

    kprintf("[INIT] Mounting initramfs root filesystem...\n");
    initramfs_init();

    kprintf("[INIT] Security Capabilities & Access Control...\n");
    security_init();

    kprintf("[INIT] Networking IPv4 Stack...\n");
    net_subsystem_init();

    kprintf("[INIT] Hypervisor Virtualization Engine (KVM)...\n");
    virtualization_subsystem_init();

    kprintf("[INIT] Performance Profiler & Telemetry Engine...\n");
    perf_subsystem_init();
    telemetry_init();
    sys_interface_init();
    numa_init();

    /* 5. Process Scheduler & Syscalls */
    kprintf("[INIT] Initializing Completely Fair Scheduler (CFS)...\n");
    scheduler_init();

    kprintf("[INIT] Registering System Call Interrupt Handler (INT 0x80)...\n");
    syscall_init();

    kprintf("\n[SUCCESS] Dionnex Kernel Subsystems Ready.\n");
    kprintf("Enabling Hardware Interrupts (STI)...\n\n");
    sti();

    /* Launch Interactive Dionnex Shell */
    shell_start();
}
