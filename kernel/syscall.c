/* ============================================================================
 * Dionnex Monolithic Kernel - System Call Interface Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <config.h>
#include <arch.h>
#include <kernel/syscall.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <kernel/idt.h>
#include <kernel/process.h>
#include <kernel/scheduler_process.h>
#include <kernel/elf_loader.h>
#include <kernel/initrd.h>
#include <kernel/module_loader.h>
#include <drivers/vga.h>
#include <drivers/serial.h>
#include <drivers/keyboard.h>
#include <drivers/rtc.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <fs/vfs.h>
#include <fs/pipe.h>
#include <string.h>
#include <io.h>

extern void syscall_isr_stub(void);

typedef int32_t (*syscall_fn_t)(registers_t *regs, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

static syscall_fn_t syscall_table[DIONNEX_MAX_SYSCALLS];

static int check_user_buf(uint32_t addr, uint32_t len, int write_required) {
    page_directory_t *pd = current_process ? current_process->page_directory : vmm_get_current_directory();
    return vmm_validate_user_range(pd, addr, len, write_required);
}

static int check_user_str(uint32_t addr, uint32_t max_len) {
    page_directory_t *pd = current_process ? current_process->page_directory : vmm_get_current_directory();
    return vmm_validate_user_string(pd, (const char*)(uintptr_t)addr, max_len);
}

static int32_t sys_exit_impl(registers_t *regs, uint32_t code, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)b; (void)c; (void)d; (void)e;
    process_exit((int)code);
    return 0;
}

static int32_t sys_fork_impl(registers_t *regs, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)a; (void)b; (void)c; (void)d; (void)e;
    if (!current_process || !regs) return -1;

    process_t *parent = current_process;
    process_t *child = (process_t*)kcalloc(1, sizeof(process_t));
    if (!child) return -1;

    memcpy(child, parent, sizeof(process_t));
    child->pid = parent->pid + 1000;
    child->parent = parent;
    child->in_trampoline = 0;
    child->list_next = NULL;
    child->sched_next = NULL;

    child->page_directory = vmm_clone_address_space(parent->page_directory);
    if (!child->page_directory) {
        kfree(child);
        return -1;
    }

    child->kernel_stack = (uint32_t*)kmalloc_aligned(8192, 16);
    if (!child->kernel_stack) {
        vmm_destroy_address_space(child->page_directory);
        kfree(child);
        return -1;
    }
    memset(child->kernel_stack, 0, 8192);
    child->esp0 = (uint32_t)child->kernel_stack + 8192U;
    child->user_eip = regs->eip;
    child->user_esp = regs->useresp;
    child->state = PROC_READY;

    sched_proc_add(child);
    return (int32_t)child->pid;
}

static int32_t sys_read_impl(registers_t *regs, uint32_t fd, uint32_t buf_addr, uint32_t len, uint32_t d, uint32_t e) {
    (void)regs; (void)d; (void)e;
    if (len == 0) return 0;
    if (!check_user_buf(buf_addr, len, 1)) return -1;

    char *out = (char*)(uintptr_t)buf_addr;

    if (current_process) {
        fd_entry_t *fde = process_get_fd(current_process, (int)fd);
        if (!fde) return -1;
        if (fde->is_console && fd == 0) {
            cpu_sti();
            uint32_t read_bytes = 0;
            while (read_bytes < len) {
                char c = 0;
                if (keyboard_has_input()) {
                    c = keyboard_read();
                } else if (serial_has_input()) {
                    c = serial_getchar();
                    if (c == '\r') c = '\n';
                } else {
                    if (read_bytes > 0) break;
                    asm volatile("hlt");
                    continue;
                }
                out[read_bytes++] = c;
                if (c == '\n' || c == '\r') break;
            }
            return (int32_t)read_bytes;
        }
        if (fde->node) {
            int n = vfs_read(fde->node, fde->offset, len, (uint8_t*)out);
            if (n > 0) fde->offset += (uint32_t)n;
            return n;
        }
        return -1;
    }
    return -1;
}

static int32_t sys_write_impl(registers_t *regs, uint32_t fd, uint32_t buf_addr, uint32_t len, uint32_t d, uint32_t e) {
    (void)regs; (void)d; (void)e;
    if (len == 0) return 0;
    if (!check_user_buf(buf_addr, len, 0)) return -1;

    const char *s = (const char*)(uintptr_t)buf_addr;

    if (current_process) {
        fd_entry_t *fde = process_get_fd(current_process, (int)fd);
        if (!fde) return -1;
        if (fde->is_console && (fd == 1 || fd == 2)) {
            for (uint32_t i = 0; i < len; i++) {
                vga_putchar(s[i]);
                if (s[i] == '\n') serial_putchar('\r');
                serial_putchar(s[i]);
            }
            return (int32_t)len;
        }
        if (fde->node) {
            int n = vfs_write(fde->node, fde->offset, len, (const uint8_t*)s);
            if (n > 0) fde->offset += (uint32_t)n;
            return n;
        }
        return -1;
    }
    return -1;
}

static int32_t sys_open_impl(registers_t *regs, uint32_t path_addr, uint32_t flags, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)c; (void)d; (void)e;
    if (!current_process || !check_user_str(path_addr, 256)) return -1;

    const char *path = (const char*)(uintptr_t)path_addr;
    vfs_node_t *node = vfs_open(path);
    if (!node) return -1;

    int fd = process_alloc_fd(current_process, node, flags);
    if (fd < 0) {
        vfs_close(node);
        return -1;
    }
    return fd;
}

static int32_t sys_close_impl(registers_t *regs, uint32_t fd, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)b; (void)c; (void)d; (void)e;
    if (!current_process || fd < 3) return -1;
    return process_close_fd(current_process, (int)fd);
}

static int32_t sys_lseek_impl(registers_t *regs, uint32_t fd, uint32_t offset, uint32_t whence, uint32_t d, uint32_t e) {
    (void)regs; (void)d; (void)e;
    if (!current_process) return -1;
    fd_entry_t *fde = process_get_fd(current_process, (int)fd);
    if (!fde || fde->is_console || !fde->node) return -1;

    int32_t new_off = (int32_t)fde->offset;
    if (whence == 0) new_off = (int32_t)offset;
    else if (whence == 1) new_off += (int32_t)offset;
    else if (whence == 2) new_off = (int32_t)fde->node->size + (int32_t)offset;
    else return -1;

    if (new_off < 0) return -1;
    fde->offset = (uint32_t)new_off;
    return new_off;
}

static int32_t sys_stat_impl(registers_t *regs, uint32_t path_addr, uint32_t stat_addr, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)c; (void)d; (void)e;
    if (!check_user_str(path_addr, 256)) return -1;
    if (!check_user_buf(stat_addr, sizeof(vfs_stat_t), 1)) return -1;
    return vfs_stat((const char*)(uintptr_t)path_addr, (vfs_stat_t*)(uintptr_t)stat_addr);
}

static int32_t sys_pipe_impl(registers_t *regs, uint32_t fds_addr, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)b; (void)c; (void)d; (void)e;
    if (!current_process || !check_user_buf(fds_addr, sizeof(int32_t) * 2U, 1)) return -1;

    vfs_node_t *rnode = NULL, *wnode = NULL;
    if (pipe_create(&rnode, &wnode) != 0) return -1;

    int rfd = process_alloc_fd(current_process, rnode, 0);
    int wfd = process_alloc_fd(current_process, wnode, 1);
    if (rfd < 0 || wfd < 0) {
        if (rfd >= 0) process_close_fd(current_process, rfd); else vfs_close(rnode);
        if (wfd >= 0) process_close_fd(current_process, wfd); else vfs_close(wnode);
        return -1;
    }

    int32_t *user_fds = (int32_t*)(uintptr_t)fds_addr;
    user_fds[0] = rfd;
    user_fds[1] = wfd;
    return 0;
}

static int32_t sys_waitpid_impl(registers_t *regs, uint32_t pid, uint32_t status_addr, uint32_t options, uint32_t d, uint32_t e) {
    (void)regs; (void)options; (void)d; (void)e;
    process_t *target = process_get_by_pid(pid);
    if (!target || target == current_process) return -1;
    if (status_addr && !check_user_buf(status_addr, sizeof(int32_t), 1)) return -1;

    cpu_sti();
    while (target->state != PROC_ZOMBIE && target->state != PROC_DEAD) {
        timer_sleep(10);
    }

    if (status_addr) {
        *(int32_t*)(uintptr_t)status_addr = (int32_t)target->exit_code;
    }
    process_destroy(target);
    return (int32_t)pid;
}

static int32_t sys_exec_impl(registers_t *regs, uint32_t path_addr, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)b; (void)c; (void)d; (void)e;
    if (!current_process || !regs || !check_user_str(path_addr, 256)) return -1;

    const char *path = (const char*)(uintptr_t)path_addr;
    char path_copy[128];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    uint32_t size = 0;
    const uint8_t *data = initrd_get_file(path_copy, &size);
    if (!data || !elf_validate(data, size)) {
        return -1;
    }

    page_directory_t *new_pd = vmm_create_address_space();
    if (!new_pd) return -1;

    uint32_t stack_pages = PROC_STACK_SIZE / PAGE_SIZE;
    uint32_t stack_bottom = PROC_STACK_BASE - PROC_STACK_SIZE;
    for (uint32_t p = 0; p < stack_pages; p++) {
        uint32_t paddr = pmm_alloc_page();
        if (!paddr) { vmm_destroy_address_space(new_pd); return -1; }
        vmm_zero_phys_page(paddr);
        vmm_map_page_in(new_pd, stack_bottom + p * PAGE_SIZE, paddr, VMM_PRESENT | VMM_WRITABLE | VMM_USER);
    }

    if (elf_load_segments(data, size, new_pd) != 0) {
        vmm_destroy_address_space(new_pd);
        return -1;
    }

    page_directory_t *old_pd = current_process->page_directory;
    current_process->page_directory = new_pd;
    vmm_switch_directory(new_pd);
    if (old_pd) vmm_destroy_address_space(old_pd);

    current_process->entry_point = elf_get_entry(data, size);
    current_process->user_eip = current_process->entry_point;
    current_process->user_esp = PROC_STACK_BASE - 16U;
    strncpy(current_process->name, path_copy, PROC_NAME_LEN - 1);

    regs->eip = current_process->user_eip;
    regs->useresp = current_process->user_esp;
    return 0;
}

static int32_t sys_sbrk_impl(registers_t *regs, uint32_t increment, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)b; (void)c; (void)d; (void)e;
    if (!current_process) return -1;

    uint32_t old_brk = current_process->brk;
    if (increment == 0) return (int32_t)old_brk;

    uint64_t new_brk = (uint64_t)old_brk + increment;
    if (new_brk < old_brk || new_brk >= PROC_MMAP_BASE) return -1;

    if (new_brk <= current_process->brk_max) {
        current_process->brk = (uint32_t)new_brk;
        return (int32_t)old_brk;
    }

    uint32_t extra = (uint32_t)(new_brk - current_process->brk_max);
    uint32_t new_pages = (extra + PAGE_SIZE - 1U) / PAGE_SIZE;

    for (uint32_t i = 0; i < new_pages; i++) {
        uint32_t paddr = pmm_alloc_page();
        if (!paddr) return -1;
        vmm_zero_phys_page(paddr);
        if (vmm_map_page_in(current_process->page_directory,
                            current_process->brk_max + (i * PAGE_SIZE),
                            paddr, VMM_PRESENT | VMM_WRITABLE | VMM_USER) != 0) {
            pmm_free_page(paddr);
            return -1;
        }
    }

    current_process->brk_max += new_pages * PAGE_SIZE;
    current_process->brk = (uint32_t)new_brk;
    return (int32_t)old_brk;
}

static int32_t sys_getpid_impl(registers_t *regs, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)a; (void)b; (void)c; (void)d; (void)e;
    return current_process ? (int32_t)current_process->pid : 0;
}

static int32_t sys_getppid_impl(registers_t *regs, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)a; (void)b; (void)c; (void)d; (void)e;
    if (current_process && current_process->parent) {
        return (int32_t)current_process->parent->pid;
    }
    return 0;
}

static int32_t sys_kill_impl(registers_t *regs, uint32_t pid, uint32_t sig, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)c; (void)d; (void)e;
    process_t *target = process_get_by_pid(pid);
    if (!target) return -1;
    if (sig >= 32) return -1;

    if (sig == SIGKILL || sig == SIGTERM) {
        if (target == current_process) {
            process_exit(128 + (int)sig);
            return 0;
        }
        target->state = PROC_ZOMBIE;
        target->exit_code = 128U + sig;
        return 0;
    }
    target->pending_signals |= (1U << sig);
    return 0;
}

static int32_t sys_mmap_impl(registers_t *regs, uint32_t addr, uint32_t len, uint32_t prot, uint32_t flags, uint32_t e) {
    (void)regs; (void)flags; (void)e;
    if (!current_process || len == 0) return -1;

    uint32_t pages = (len + PAGE_SIZE - 1U) / PAGE_SIZE;
    uint32_t start_vaddr = addr ? ALIGN_DOWN(addr, PAGE_SIZE) : current_process->mmap_next;

    if (start_vaddr < DIONNEX_USER_VADDR_MIN ||
        (uint64_t)start_vaddr + ((uint64_t)pages * PAGE_SIZE) > PROC_STACK_BASE - PROC_STACK_SIZE) {
        return -1;
    }

    uint32_t pflags = VMM_PRESENT | VMM_USER;
    if (prot & 0x2) pflags |= VMM_WRITABLE;

    for (uint32_t i = 0; i < pages; i++) {
        uint32_t vpage = start_vaddr + (i * PAGE_SIZE);
        if (vmm_get_phys_in(current_process->page_directory, vpage) != 0) {
            return -1; /* Already mapped */
        }
    }

    for (uint32_t i = 0; i < pages; i++) {
        uint32_t paddr = pmm_alloc_page();
        if (!paddr) return -1;
        vmm_zero_phys_page(paddr);
        vmm_map_page_in(current_process->page_directory, start_vaddr + (i * PAGE_SIZE), paddr, pflags);
    }

    if (!addr) {
        current_process->mmap_next = start_vaddr + (pages * PAGE_SIZE);
    }
    return (int32_t)start_vaddr;
}

static int32_t sys_munmap_impl(registers_t *regs, uint32_t addr, uint32_t len, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)c; (void)d; (void)e;
    if (!current_process || len == 0) return -1;
    if (addr < DIONNEX_USER_VADDR_MIN) return -1;
    uint64_t end = (uint64_t)addr + len;
    if (end > DIONNEX_USER_VADDR_MAX || end < addr) return -1;

    uint32_t pages = (len + PAGE_SIZE - 1U) / PAGE_SIZE;
    for (uint32_t i = 0; i < pages; i++) {
        uint32_t vpage = ALIGN_DOWN(addr, PAGE_SIZE) + (i * PAGE_SIZE);
        uint32_t phys = vmm_get_phys_in(current_process->page_directory, vpage) & ~0xFFFU;
        if (phys) {
            vmm_unmap_page_in(current_process->page_directory, vpage);
            pmm_free_page(phys);
        }
    }
    return 0;
}

static int32_t sys_gettimeofday_impl(registers_t *regs, uint32_t tv_addr, uint32_t tz, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)tz; (void)c; (void)d; (void)e;
    if (!check_user_buf(tv_addr, 8, 1)) return -1;
    uint32_t *tv = (uint32_t*)(uintptr_t)tv_addr;
    tv[0] = rtc_get_ticks();
    tv[1] = (timer_get_ticks() % 1000U) * 1000U;
    return 0;
}

static int32_t sys_uname_impl(registers_t *regs, uint32_t buf_addr, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)b; (void)c; (void)d; (void)e;
    if (!check_user_buf(buf_addr, 195, 1)) return -1;
    char *buf = (char*)(uintptr_t)buf_addr;
    memset(buf, 0, 195);
    strncpy(buf, "Dionnex", 64);
    strncpy(buf + 65, DIONNEX_VERSION, 64);
    strncpy(buf + 130, DIONNEX_ARCH_NAME, 64);
    return 0;
}

static int32_t sys_nanosleep_impl(registers_t *regs, uint32_t ms, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)b; (void)c; (void)d; (void)e;
    cpu_sti();
    timer_sleep(ms);
    return 0;
}

static int32_t sys_init_module_impl(registers_t *regs, uint32_t buf_addr, uint32_t size, uint32_t name_addr, uint32_t d, uint32_t e) {
    (void)regs; (void)d; (void)e;
    if (size == 0 || size > (4U * 1024U * 1024U)) return -1;
    if (!check_user_buf(buf_addr, size, 0)) return -1;
    if (!check_user_str(name_addr, 64)) return -1;
    return module_load_binary((const char*)(uintptr_t)name_addr, (const uint8_t*)(uintptr_t)buf_addr, size);
}

static int32_t sys_delete_module_impl(registers_t *regs, uint32_t name_addr, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)regs; (void)b; (void)c; (void)d; (void)e;
    if (!check_user_str(name_addr, 64)) return -1;
    return module_unload_binary((const char*)(uintptr_t)name_addr);
}

static int32_t sys_query_module_impl(registers_t *regs, uint32_t name_addr, uint32_t buf_addr, uint32_t max_len, uint32_t d, uint32_t e) {
    (void)regs; (void)name_addr; (void)d; (void)e;
    if (!check_user_buf(buf_addr, max_len, 1)) return -1;
    return module_get_proc_info((char*)(uintptr_t)buf_addr, max_len);
}

void syscall_handler(registers_t *regs) {
    if (!regs) return;

    uint32_t num = regs->eax;
    if (num < DIONNEX_MAX_SYSCALLS && syscall_table[num]) {
        regs->eax = (uint32_t)syscall_table[num](regs, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
    } else {
        printk("Syscall: Invalid syscall number %u\n", num);
        regs->eax = (uint32_t)-1;
    }
}

void syscall_init(void) {
    for (int i = 0; i < DIONNEX_MAX_SYSCALLS; i++) {
        syscall_table[i] = NULL;
    }

    syscall_table[SYS_EXIT]          = sys_exit_impl;
    syscall_table[SYS_FORK]          = sys_fork_impl;
    syscall_table[SYS_READ]          = sys_read_impl;
    syscall_table[SYS_WRITE]         = sys_write_impl;
    syscall_table[SYS_OPEN]          = sys_open_impl;
    syscall_table[SYS_CLOSE]         = sys_close_impl;
    syscall_table[SYS_WAITPID]       = sys_waitpid_impl;
    syscall_table[SYS_EXEC]          = sys_exec_impl;
    syscall_table[SYS_SBRK]          = sys_sbrk_impl;
    syscall_table[SYS_STAT]          = sys_stat_impl;
    syscall_table[SYS_LSEEK]         = sys_lseek_impl;
    syscall_table[SYS_GETPID]        = sys_getpid_impl;
    syscall_table[SYS_GETPPID]       = sys_getppid_impl;
    syscall_table[SYS_KILL]          = sys_kill_impl;
    syscall_table[SYS_MMAP]          = sys_mmap_impl;
    syscall_table[SYS_MUNMAP]        = sys_munmap_impl;
    syscall_table[SYS_PIPE]          = sys_pipe_impl;
    syscall_table[SYS_GETTIMEOFDAY]  = sys_gettimeofday_impl;
    syscall_table[SYS_UNAME]         = sys_uname_impl;
    syscall_table[SYS_NANOSLEEP]     = sys_nanosleep_impl;
    syscall_table[SYS_INIT_MODULE]   = sys_init_module_impl;
    syscall_table[SYS_DELETE_MODULE] = sys_delete_module_impl;
    syscall_table[SYS_QUERY_MODULE]  = sys_query_module_impl;
    syscall_table[SYS_EXIT_GROUP]    = sys_exit_impl;

    idt_set_gate(0x80, (uint32_t)syscall_isr_stub, 0x08, 0xEE);
    printk("Syscall: System call interface initialized (int 0x80, validated)\n");
}
