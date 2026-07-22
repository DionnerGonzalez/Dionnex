/* ============================================================================
 * Dionnex Kernel - Round-Robin Task Scheduler (process/scheduler.c)
 * ============================================================================
 */
#include <kernel.h>

extern void switch_to(uint32_t** old_esp, uint32_t* new_esp);

struct task_struct {
    uint32_t pid;
    char name[32];
    uint32_t state;
    uint32_t esp;
    uint32_t cr3;
    uint32_t ring;
    uint8_t stack[4096];
    struct task_struct* next;
};

static struct task_struct task_idle;
static struct task_struct task_worker1;
static struct task_struct task_worker2;

static struct task_struct* current_task = &task_idle;

void scheduler_init(void) {
    task_idle.pid = 0;
    task_idle.state = 0; // RUNNING
    task_idle.esp = 0;
    
    // Copy name
    const char* idle_name = "kthread_idle";
    for(int i=0; i<12; i++) task_idle.name[i] = idle_name[i];
    task_idle.name[12] = 0;

    const char* w1_name = "task_worker_1";
    for(int i=0; i<13; i++) task_worker1.name[i] = w1_name[i];
    task_worker1.name[13] = 0;
    task_worker1.pid = 1;
    task_worker1.state = 1; // READY

    const char* w2_name = "task_worker_2";
    for(int i=0; i<13; i++) task_worker2.name[i] = w2_name[i];
    task_worker2.name[13] = 0;
    task_worker2.pid = 2;
    task_worker2.state = 1; // READY

    task_idle.next = &task_worker1;
    task_worker1.next = &task_worker2;
    task_worker2.next = &task_idle;

    current_task = &task_idle;

    vga_puts("[SCHED] Planificador Round-Robin listo (3 tareas alternando: idle, worker1, worker2).\n");
}

void schedule(void) {
    struct task_struct* prev = current_task;
    struct task_struct* next = current_task->next;

    if (!next) return;

    current_task = next;
    switch_to((uint32_t**)&prev->esp, (uint32_t*)next->esp);
}
