/* ============================================================================
 * Dionnex Monolithic Kernel - Process Management Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/process.h>
#include <kernel/tss.h>
#include <kernel/elf_loader.h>
#include <kernel/scheduler_process.h>
#include <kernel/printk.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <string.h>

static process_t *proc_list = NULL;
process_t *current_process = NULL;
static uint32_t next_pid = 1;

void process_init(void) {
    proc_list = NULL;
    current_process = NULL;
    next_pid = 1;
    printk("Process: User process manager initialized\n");
}

process_t *process_create(const char *name, uint8_t *elf_data, uint32_t elf_size) {
    if (!elf_validate(elf_data, elf_size)) {
        printk("Process: Cannot create process '%s' - Invalid ELF payload\n", name ? name : "unknown");
        return NULL;
    }

    process_t *proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) return NULL;

    memset(proc, 0, sizeof(process_t));
    proc->pid = next_pid++;
    strncpy(proc->name, name ? name : "user_proc", PROC_NAME_LEN - 1);
    proc->state = PROC_NEW;

    // Allocate page directory
    proc->page_directory = (uint32_t*)pmm_alloc_page();
    if (!proc->page_directory) {
        kfree(proc);
        return NULL;
    }
    memset(proc->page_directory, 0, 4096);

    // Identity map kernel first 4MB space for kernel routines & syscalls
    for (uint32_t addr = 0; addr < 0x00400000; addr += 4096) {
        vmm_map_page(addr, addr, VMM_PRESENT | VMM_WRITABLE);
    }

    // Allocate and map 1MB User Stack space at PROC_STACK_BASE
    uint32_t stack_pages = PROC_STACK_SIZE / 4096;
    uint32_t stack_bottom = PROC_STACK_BASE - PROC_STACK_SIZE;
    for (uint32_t p = 0; p < stack_pages; p++) {
        void *paddr = pmm_alloc_page();
        if (paddr) {
            vmm_map_page(stack_bottom + (p * 4096), (uint32_t)paddr, VMM_PRESENT | VMM_WRITABLE | VMM_USER);
        }
    }

    // Allocate and map initial Heap region
    for (uint32_t p = 0; p < 16; p++) {
        void *hpaddr = pmm_alloc_page();
        if (hpaddr) {
            vmm_map_page(PROC_HEAP_BASE + (p * 4096), (uint32_t)hpaddr, VMM_PRESENT | VMM_WRITABLE | VMM_USER);
        }
    }

    proc->brk = PROC_HEAP_BASE;
    proc->brk_max = PROC_HEAP_BASE + (16 * 4096);

    // Load ELF segments into user address space
    if (elf_load_segments(elf_data, elf_size, proc->page_directory) != 0) {
        kfree(proc);
        return NULL;
    }

    proc->entry_point = elf_get_entry(elf_data);
    proc->user_eip = proc->entry_point;
    proc->user_esp = PROC_STACK_BASE - 16; // Align user stack

    // Allocate Kernel Stack for ring 3 -> ring 0 transitions (8KB)
    proc->kernel_stack = (uint32_t*)kmalloc(8192);
    if (!proc->kernel_stack) {
        kfree(proc);
        return NULL;
    }
    proc->esp0 = (uint32_t)proc->kernel_stack + 8192;

    // Initialize Standard File Descriptors
    for (int i = 0; i < PROC_MAX_FDS; i++) {
        proc->fds[i] = -1;
    }
    proc->fds[0] = 0; // stdin
    proc->fds[1] = 1; // stdout
    proc->fds[2] = 2; // stderr

    proc->state = PROC_READY;

    // Link process to list
    proc->next = proc_list;
    proc_list = proc;

    printk("Process: Created '%s' (pid %u, entry 0x%x)\n", proc->name, proc->pid, proc->entry_point);
    return proc;
}

void process_exit(int code) {
    if (!current_process) return;

    current_process->exit_code = (uint32_t)code;
    current_process->state = PROC_ZOMBIE;

    printk("Process: '%s' (pid %u) exited with code %d\n",
           current_process->name, current_process->pid, code);

    schedule_process();
}

void process_destroy(process_t *proc) {
    if (!proc) return;

    // Remove from list
    if (proc_list == proc) {
        proc_list = proc->next;
    } else if (proc_list) {
        process_t *curr = proc_list;
        while (curr->next && curr->next != proc) {
            curr = curr->next;
        }
        if (curr->next == proc) {
            curr->next = proc->next;
        }
    }

    if (proc->kernel_stack) kfree(proc->kernel_stack);
    proc->state = PROC_DEAD;
    kfree(proc);
}

process_t *process_get_current(void) {
    return current_process;
}

process_t *process_get_by_pid(uint32_t pid) {
    process_t *curr = proc_list;
    while (curr) {
        if (curr->pid == pid) return curr;
        curr = curr->next;
    }
    return NULL;
}

uint32_t process_get_count(void) {
    uint32_t count = 0;
    process_t *curr = proc_list;
    while (curr) {
        count++;
        curr = curr->next;
    }
    return count;
}

void process_list(void) {
    printk("PID  NAME             STATE      VRUNTIME   EXIT\n");
    printk("---  ---------------  ---------  ---------  ----\n");

    process_t *curr = proc_list;
    while (curr) {
        const char *st_str = "NEW";
        switch (curr->state) {
            case PROC_READY:   st_str = "READY"; break;
            case PROC_RUNNING: st_str = "RUNNING"; break;
            case PROC_BLOCKED: st_str = "BLOCKED"; break;
            case PROC_ZOMBIE:  st_str = "ZOMBIE"; break;
            case PROC_DEAD:    st_str = "DEAD"; break;
            default: break;
        }

        printk("%-4u %-16s %-10s %-10u %-4u\n",
               curr->pid, curr->name, st_str, curr->vruntime, curr->exit_code);
        curr = curr->next;
    }
}
