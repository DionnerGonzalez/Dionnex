/*
 * Dionnex Main System Initialization Sequence
 */

#include <kernel/core/kernel.h>

extern void pmm_init(void);
extern void vmm_init(void);
extern void sched_init(void);
extern void syscall_init(void);
extern void irq_init(void);
extern int drivers_subsystem_init(void);

asmlinkage void __init start_kernel(void)
{
	pr_info("Booting %s...\n", get_kernel_banner());

	/* Phase 1: Core Physical Memory & Page Frame Allocator */
	pmm_init();

	/* Phase 2: Virtual Memory Manager & Slab Allocator */
	vmm_init();

	/* Phase 3: Interrupt Vectors & APIC Initialization */
	irq_init();

	/* Phase 4: Modular Hardware Drivers Initialization */
	drivers_subsystem_init();

	/* Phase 5: Scheduler & Initial Task0 Creation */
	sched_init();

	/* Phase 6: System Call Entry Point Wiring */
	syscall_init();

	pr_info("Dionnex Kernel Subsystems Initialized Successfully. Spawning init process.\n");
}
