/*
 * Dionnex Main System Initialization Sequence
 */

#include <kernel/core/kernel.h>

asmlinkage void __init start_kernel(void)
{
	pr_info("Booting Dionnex Kernel v1.0.0...\n");

	/* Phase 1: Core Physical Memory & Page Frame Allocator */
	pmm_init(128);

	/* Phase 2: Virtual Memory Manager & Slab Allocator */
	vmm_init();

	/* Phase 3: Interrupt Vectors & APIC Initialization */
	gdt_init();
	idt_init();
	pic_remap();
	pit_init(100);

	/* Phase 4: Modular Hardware Drivers Initialization */
	drivers_subsystem_init();

	/* Phase 5: Scheduler & Initial Task0 Creation */
	scheduler_init();

	/* Phase 6: System Call Entry Point Wiring */
	syscall_init();

	pr_info("Dionnex Kernel Subsystems Initialized Successfully.\n");
}

