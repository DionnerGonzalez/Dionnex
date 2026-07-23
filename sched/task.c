/* ============================================================================
 * Dionnex Monolithic Kernel - Task Creation & Execution Management
 * ============================================================================
 */
#include <sched/task.h>
#include <sched/scheduler.h>
#include <mm/pmm.h>
#include <kernel/printk.h>

task_t *current_task = NULL;
task_t *task_list_head = NULL;
static uint32_t next_pid = 0;
static task_t kernel_task;

static void strncpy_manual(char *dst, const char *src, size_t max_len) {
    size_t i = 0;
    while (src && src[i] && i < max_len - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

void task_init(void) {
    kernel_task.pid = 0;
    strncpy_manual(kernel_task.name, "kernel_idle", 32);
    kernel_task.state = TASK_RUNNING;
    kernel_task.esp = 0;
    kernel_task.stack_base = NULL;
    kernel_task.vruntime = 0;
    kernel_task.priority = 0;
    kernel_task.next = NULL;

    current_task = &kernel_task;
    task_list_head = &kernel_task;
    next_pid = 1;

    printk("Task: Kernel idle task created (pid 0)\n");
}

task_t *task_create(const char *name, void (*entry)(void), uint32_t priority) {
    task_t *task = (task_t*)pmm_alloc_page();
    if (!task) return NULL;

    task->pid = next_pid++;
    strncpy_manual(task->name, name ? name : "task", 32);
    task->state = TASK_READY;
    task->vruntime = 0;
    task->priority = priority;
    task->next = NULL;

    /* Allocate 8KB kernel stack (2 contiguous pages) */
    task->stack_base = (uint32_t*)pmm_alloc_page();
    if (!task->stack_base) {
        pmm_free_page((uint32_t)task);
        return NULL;
    }
    pmm_alloc_page(); // Alloc second page for 8KB

    uint32_t *stack_top = (uint32_t*)((uint32_t)task->stack_base + TASK_STACK_SIZE);

    /* Initial stack frame for context_switch (popf then popa) */
    *(--stack_top) = (uint32_t)entry; // Return EIP
    *(--stack_top) = 0x202;            // EFLAGS (IF enabled)

    /* PUSHA frame (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI) */
    *(--stack_top) = 0; // EAX
    *(--stack_top) = 0; // ECX
    *(--stack_top) = 0; // EDX
    *(--stack_top) = 0; // EBX
    *(--stack_top) = 0; // ESP
    *(--stack_top) = 0; // EBP
    *(--stack_top) = 0; // ESI
    *(--stack_top) = 0; // EDI

    task->esp = (uint32_t)stack_top;

    /* Append to task list */
    task_t *curr = task_list_head;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = task;

    printk("Task: Created '%s' (pid %u)\n", task->name, task->pid);
    return task;
}

void task_exit(void) {
    if (current_task) {
        current_task->state = TASK_DEAD;
        schedule();
    }
}

task_t *task_get_current(void) {
    return current_task;
}

void task_yield(void) {
    schedule();
}
