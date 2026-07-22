/* ============================================================================
 * Dionnex Kernel - Process & Task Control Block Structure (process/task.c)
 * ============================================================================
 */
#include <kernel.h>

typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_ZOMBIE
} task_state_t;

struct task_struct {
    uint32_t pid;
    char name[32];
    task_state_t state;
    uint32_t esp;
    uint32_t cr3;
    uint32_t ring; // 0 = Kernel, 3 = User
    uint8_t stack[4096];
    struct task_struct* next;
};

static struct task_struct task_list[8];
static uint32_t next_pid = 1;

void task_alpha_entry(void) {
    while (1) {
        // Task A execution
        for (volatile int i = 0; i < 100000; i++);
    }
}

void task_beta_entry(void) {
    while (1) {
        // Task B execution
        for (volatile int i = 0; i < 100000; i++);
    }
}

struct task_struct* create_kernel_task(const char* name, void (*entry)(void)) {
    if (next_pid >= 8) return NULL;

    struct task_struct* t = &task_list[next_pid];
    t->pid = next_pid++;
    
    // Copy name safely
    size_t i = 0;
    while (name[i] && i < 31) {
        t->name[i] = name[i];
        i++;
    }
    t->name[i] = '\0';

    t->state = TASK_READY;
    t->ring = 0;

    // Prepare initial stack for context switch
    uint32_t* esp = (uint32_t*)&t->stack[4096 - 16];
    *(--esp) = (uint32_t)entry; // Return EIP
    *(--esp) = 0; // EBP
    *(--esp) = 0; // EBX
    *(--esp) = 0; // ESI
    *(--esp) = 0; // EDI

    t->esp = (uint32_t)esp;
    return t;
}
