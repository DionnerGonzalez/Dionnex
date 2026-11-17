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

extern void ring3_trampoline_enter(uint32_t *saved_kesp, uint32_t eip, uint32_t user_esp, uint32_t cr3);
extern void ring3_trampoline_exit(uint32_t saved_kesp, uint32_t kernel_cr3);

static process_t *proc_list = NULL;
process_t *current_process = NULL;
static uint32_t next_pid = 1;

void process_init(void) {
    proc_list = NULL;
    current_process = NULL;
    next_pid = 1;
    printk("Process: User process manager initialized\n");
}

static void process_init_fds(process_t *proc) {
    for (int i = 0; i < PROC_MAX_FDS; i++) {
        proc->fd_table[i].used = 0;
        proc->fd_table[i].is_console = 0;
        proc->fd_table[i].flags = 0;
        proc->fd_table[i].offset = 0;
        proc->fd_table[i].node = NULL;
    }
    for (int i = 0; i < 3; i++) {
        proc->fd_table[i].used = 1;
        proc->fd_table[i].is_console = 1;
    }
}

int process_alloc_fd(process_t *proc, vfs_node_t *node, uint32_t flags) {
    if (!proc || !node) return -1;
    for (int i = 3; i < PROC_MAX_FDS; i++) {
        if (!proc->fd_table[i].used) {
            proc->fd_table[i].used = 1;
            proc->fd_table[i].is_console = 0;
            proc->fd_table[i].flags = flags;
            proc->fd_table[i].offset = 0;
            proc->fd_table[i].node = node;
            return i;
        }
    }
    return -1;
}

fd_entry_t *process_get_fd(process_t *proc, int fd) {
    if (!proc || fd < 0 || fd >= PROC_MAX_FDS) return NULL;
    if (!proc->fd_table[fd].used) return NULL;
    return &proc->fd_table[fd];
}

int process_close_fd(process_t *proc, int fd) {
    if (!proc || fd < 0 || fd >= PROC_MAX_FDS) return -1;
    if (!proc->fd_table[fd].used) return -1;
    if (!proc->fd_table[fd].is_console && proc->fd_table[fd].node) {
        vfs_close(proc->fd_table[fd].node);
    }
    proc->fd_table[fd].used = 0;
    proc->fd_table[fd].is_console = 0;
    proc->fd_table[fd].node = NULL;
    proc->fd_table[fd].offset = 0;
    return 0;
}

process_t *process_create(const char *name, const uint8_t *elf_data, uint32_t elf_size) {
    if (!elf_validate(elf_data, elf_size)) {
        printk("Process: Cannot create process '%s' - Invalid ELF payload\n", name ? name : "unknown");
        return NULL;
    }

    process_t *proc = (process_t*)kcalloc(1, sizeof(process_t));
    if (!proc) return NULL;

    proc->pid = next_pid++;
    strncpy(proc->name, name ? name : "user_proc", PROC_NAME_LEN - 1);
    proc->state = PROC_NEW;

    /* Create isolated per-process page directory */
    proc->page_directory = vmm_create_address_space();
    if (!proc->page_directory) {
        kfree(proc);
        return NULL;
    }

    /* Map user stack at [PROC_STACK_BASE - PROC_STACK_SIZE, PROC_STACK_BASE) */
    uint32_t stack_pages = PROC_STACK_SIZE / PAGE_SIZE;
    uint32_t stack_bottom = PROC_STACK_BASE - PROC_STACK_SIZE;
    for (uint32_t p = 0; p < stack_pages; p++) {
        uint32_t paddr = pmm_alloc_page();
        if (!paddr) {
            vmm_destroy_address_space(proc->page_directory);
            kfree(proc);
            return NULL;
        }
        vmm_zero_phys_page(paddr);
        if (vmm_map_page_in(proc->page_directory, stack_bottom + (p * PAGE_SIZE),
                            paddr, VMM_PRESENT | VMM_WRITABLE | VMM_USER) != 0) {
            pmm_free_page(paddr);
            vmm_destroy_address_space(proc->page_directory);
            kfree(proc);
            return NULL;
        }
    }

    /* Map initial user heap (16 pages = 64KB) */
    for (uint32_t p = 0; p < 16; p++) {
        uint32_t hpaddr = pmm_alloc_page();
        if (!hpaddr) {
            vmm_destroy_address_space(proc->page_directory);
            kfree(proc);
            return NULL;
        }
        vmm_zero_phys_page(hpaddr);
        if (vmm_map_page_in(proc->page_directory, PROC_HEAP_BASE + (p * PAGE_SIZE),
                            hpaddr, VMM_PRESENT | VMM_WRITABLE | VMM_USER) != 0) {
            pmm_free_page(hpaddr);
            vmm_destroy_address_space(proc->page_directory);
            kfree(proc);
            return NULL;
        }
    }

    proc->brk = PROC_HEAP_BASE;
    proc->brk_max = PROC_HEAP_BASE + (16U * PAGE_SIZE);
    proc->mmap_next = PROC_MMAP_BASE;

    /* Load ELF segments into isolated address space */
    if (elf_load_segments(elf_data, elf_size, proc->page_directory) != 0) {
        vmm_destroy_address_space(proc->page_directory);
        kfree(proc);
        return NULL;
    }

    proc->entry_point = elf_get_entry(elf_data, elf_size);
    proc->user_eip = proc->entry_point;
    proc->user_esp = PROC_STACK_BASE - 16U;

    /* Allocate 8KB Kernel Stack for Ring 3 -> Ring 0 traps/syscalls */
    proc->kernel_stack = (uint32_t*)kmalloc_aligned(8192, 16);
    if (!proc->kernel_stack) {
        vmm_destroy_address_space(proc->page_directory);
        kfree(proc);
        return NULL;
    }
    memset(proc->kernel_stack, 0, 8192);
    proc->esp0 = (uint32_t)proc->kernel_stack + 8192U;

    process_init_fds(proc);
    proc->state = PROC_READY;

    proc->list_next = proc_list;
    proc_list = proc;

    printk("Process: Created '%s' (pid %u, cr3 0x%x, entry 0x%x)\n",
           proc->name, proc->pid, (uint32_t)proc->page_directory, proc->entry_point);
    return proc;
}

int process_run(process_t *proc) {
    if (!proc || !proc->page_directory) return -1;

    process_t *prev = current_process;
    current_process = proc;
    proc->state = PROC_RUNNING;
    proc->in_trampoline = 1;

    tss_set_kernel_stack(proc->esp0);
    vmm_switch_directory(proc->page_directory);

    ring3_trampoline_enter(&proc->kctx_esp, proc->user_eip, proc->user_esp,
                           (uint32_t)proc->page_directory);

    /* Resumes right here when process calls process_exit() */
    vmm_switch_directory(vmm_get_kernel_directory());
    current_process = prev;
    if (prev) {
        tss_set_kernel_stack(prev->esp0);
    }
    proc->in_trampoline = 0;
    return (int)proc->exit_code;
}

void process_exit(int code) {
    if (!current_process) return;

    process_t *proc = current_process;
    proc->exit_code = (uint32_t)code;
    proc->state = PROC_ZOMBIE;

    printk("Process: '%s' (pid %u) exited with code %d\n",
           proc->name, proc->pid, code);

    if (proc->in_trampoline) {
        uint32_t k_esp = proc->kctx_esp;
        uint32_t k_cr3 = (uint32_t)vmm_get_kernel_directory();
        ring3_trampoline_exit(k_esp, k_cr3);
    }

    schedule_process();
}

void process_destroy(process_t *proc) {
    if (!proc) return;

    sched_proc_remove(proc);

    if (proc_list == proc) {
        proc_list = proc->list_next;
    } else if (proc_list) {
        process_t *curr = proc_list;
        while (curr->list_next && curr->list_next != proc) {
            curr = curr->list_next;
        }
        if (curr->list_next == proc) {
            curr->list_next = proc->list_next;
        }
    }

    for (int i = 0; i < PROC_MAX_FDS; i++) {
        process_close_fd(proc, i);
    }

    if (proc->page_directory) {
        vmm_destroy_address_space(proc->page_directory);
        proc->page_directory = NULL;
    }
    if (proc->kernel_stack) {
        kfree(proc->kernel_stack);
        proc->kernel_stack = NULL;
    }
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
        curr = curr->list_next;
    }
    return NULL;
}

uint32_t process_get_count(void) {
    uint32_t count = 0;
    process_t *curr = proc_list;
    while (curr) {
        count++;
        curr = curr->list_next;
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
        curr = curr->list_next;
    }
}
