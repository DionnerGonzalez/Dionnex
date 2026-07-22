/* ============================================================================
 * Dionnex Kernel - Process Control Block (PCB) Management (process/task.c)
 * ============================================================================
 */
#include <kernel/kernel.h>
#include <kernel/kabi.h>

static uint32_t next_pid = 1;
static struct task_struct idle_task;
static struct task_struct* current_task = &idle_task;

void task_init(void) {
    idle_task.pid = 0;
    strcpy(idle_task.name, "idle");
    idle_task.state = TASK_RUNNING;
    idle_task.priority = 0;
    idle_task.vruntime = 0;
    idle_task.stack = NULL;
    idle_task.next = &idle_task;
}

struct task_struct* create_task(const char* name, void (*entry)(void), uint32_t priority) {
    struct task_struct* task = (struct task_struct*)kmalloc(sizeof(struct task_struct));
    if (!task) return NULL;

    task->pid = next_pid++;
    strncpy(task->name, name, 31);
    task->state = TASK_READY;
    task->priority = priority;
    task->vruntime = 0;

    // Allocate 8KB stack
    task->stack = kmalloc(8192);
    if (!task->stack) {
        kfree(task);
        return NULL;
    }

    uint32_t* stack_top = (uint32_t*)((uint8_t*)task->stack + 8192);

    // Push initial stack frame for task context switch
    *(--stack_top) = 0x0202; // EFLAGS (Interrupts enabled)
    *(--stack_top) = 0x08;   // CS (Kernel code selector)
    *(--stack_top) = (uint32_t)entry; // EIP
    *(--stack_top) = 0;      // EAX
    *(--stack_top) = 0;      // ECX
    *(--stack_top) = 0;      // EDX
    *(--stack_top) = 0;      // EBX
    *(--stack_top) = 0;      // ESP
    *(--stack_top) = 0;      // EBP
    *(--stack_top) = 0;      // ESI
    *(--stack_top) = 0;      // EDI

    task->esp = (uint32_t)stack_top;
    task->next = NULL;

    return task;
}

struct task_struct* get_current_task(void) {
    return current_task;
}

void set_current_task(struct task_struct* task) {
    current_task = task;
}
