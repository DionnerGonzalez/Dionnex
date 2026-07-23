/* ============================================================================
 * Dionnex Monolithic Kernel - Process Scheduler Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/scheduler_process.h>
#include <kernel/process.h>
#include <kernel/tss.h>
#include <kernel/printk.h>

static process_t *sched_queue = NULL;

void sched_proc_init(void) {
    sched_queue = NULL;
    printk("Scheduler: Process preemptive scheduler initialized\n");
}

void sched_proc_add(process_t *proc) {
    if (!proc) return;
    proc->state = PROC_READY;

    proc->next = sched_queue;
    sched_queue = proc;
}

void sched_proc_remove(process_t *proc) {
    if (!proc || !sched_queue) return;

    if (sched_queue == proc) {
        sched_queue = proc->next;
        return;
    }

    process_t *curr = sched_queue;
    while (curr->next) {
        if (curr->next == proc) {
            curr->next = proc->next;
            return;
        }
        curr = curr->next;
    }
}

void enter_userspace(uint32_t eip, uint32_t esp) {
    asm volatile(
        "cli\n\t"
        "mov $0x23, %%ax\n\t" // User Data Segment (0x20 | 3)
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "pushl $0x23\n\t"     // SS
        "pushl %1\n\t"        // ESP
        "pushfl\n\t"          // EFLAGS
        "popl %%eax\n\t"
        "orl $0x200, %%eax\n\t" // Enable interrupts (IF bit)
        "pushl %%eax\n\t"
        "pushl $0x1B\n\t"     // CS User Code Segment (0x18 | 3)
        "pushl %0\n\t"        // EIP
        "iret\n\t"
        :
        : "r"(eip), "r"(esp)
        : "eax", "memory"
    );
}

void schedule_process(void) {
    if (!sched_queue) return;

    // Pick process in PROC_READY state with lowest vruntime
    process_t *best = NULL;
    process_t *curr = sched_queue;

    while (curr) {
        if (curr->state == PROC_READY) {
            if (!best || curr->vruntime < best->vruntime) {
                best = curr;
            }
        }
        curr = curr->next;
    }

    if (!best) return;

    if (current_process && current_process->state == PROC_RUNNING) {
        current_process->state = PROC_READY;
    }

    current_process = best;
    current_process->state = PROC_RUNNING;

    // Update TSS kernel stack for ring 3 -> ring 0 interrupts
    tss_set_kernel_stack(current_process->esp0);

    // If process hasn't started yet, jump into ring 3
    if (current_process->vruntime == 0) {
        current_process->vruntime = 1;
        enter_userspace(current_process->user_eip, current_process->user_esp);
    }
}

void sched_proc_tick(void) {
    if (current_process && current_process->state == PROC_RUNNING) {
        current_process->vruntime += 10;
        schedule_process();
    }
}
