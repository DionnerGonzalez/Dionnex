/* ============================================================================
 * Dionnex Monolithic Kernel - System Call Interface Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <kernel/syscall.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <kernel/idt.h>
#include <kernel/process.h>
#include <kernel/elf_loader.h>
#include <kernel/initrd.h>
#include <kernel/module_loader.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <drivers/rtc.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <sched/task.h>
#include <sched/scheduler.h>
#include <fs/vfs.h>
#include <string.h>
#include <io.h>

extern void syscall_isr_stub(void);

typedef int32_t (*syscall_fn_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

static syscall_fn_t syscall_table[DIONNEX_MAX_SYSCALLS];

static int32_t sys_exit_impl(uint32_t code, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)b; (void)c; (void)d; (void)e;
    process_exit((int)code);
    return 0;
}

static int32_t sys_fork_impl(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)a; (void)b; (void)c; (void)d; (void)e;
    if (!current_process) return -1;

    process_t *parent = current_process;
    process_t *child = (process_t*)kmalloc(sizeof(process_t));
    if (!child) return -1;

    memcpy(child, parent, sizeof(process_t));
    child->pid = parent->pid + 1000;
    child->parent = parent;

    child->kernel_stack = (uint32_t*)kmalloc(8192);
    if (child->kernel_stack) {
        memcpy(child->kernel_stack, parent->kernel_stack, 8192);
        child->esp0 = (uint32_t)child->kernel_stack + 8192;
    }

    child->state = PROC_READY;
    printk("Syscall: Forked process PID %u -> Child PID %u\n", parent->pid, child->pid);
    return (int32_t)child->pid;
}

static int32_t sys_read_impl(uint32_t fd, uint32_t buf_addr, uint32_t len, uint32_t d, uint32_t e) {
    (void)d; (void)e;
    if (!buf_addr || len == 0) return 0;

    if (fd == 0) { // stdin
        char *s = (char*)buf_addr;
        uint32_t read_bytes = 0;
        while (read_bytes < len) {
            char c = keyboard_read();
            if (c == 0) break;
            s[read_bytes++] = c;
            if (c == '\n' || c == '\r') break;
        }
        return (int32_t)read_bytes;
    }
    return -1;
}

static int32_t sys_write_impl(uint32_t fd, uint32_t buf_addr, uint32_t len, uint32_t d, uint32_t e) {
    (void)d; (void)e;
    if (!buf_addr) return -1;

    if (fd == 1 || fd == 2) { // stdout / stderr
        const char *s = (const char*)buf_addr;
        for (uint32_t i = 0; i < len; i++) {
            vga_putchar(s[i]);
        }
        return (int32_t)len;
    }
    return -1;
}

static int32_t sys_open_impl(uint32_t path_addr, uint32_t flags, uint32_t c, uint32_t d, uint32_t e) {
    (void)flags; (void)c; (void)d; (void)e;
    if (!path_addr) return -1;
    vfs_node_t *node = vfs_open((const char*)path_addr);
    return node ? (int32_t)(uintptr_t)node : -1;
}

static int32_t sys_close_impl(uint32_t handle, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)b; (void)c; (void)d; (void)e;
    vfs_node_t *node = (vfs_node_t*)(uintptr_t)handle;
    return vfs_close(node);
}

static int32_t sys_waitpid_impl(uint32_t pid, uint32_t status_addr, uint32_t options, uint32_t d, uint32_t e) {
    (void)options; (void)d; (void)e;
    process_t *target = process_get_by_pid(pid);
    if (!target) return -1;

    while (target->state != PROC_ZOMBIE && target->state != PROC_DEAD) {
        timer_sleep(10);
    }

    if (status_addr) {
        *(int*)status_addr = target->exit_code;
    }
    process_destroy(target);
    return (int32_t)pid;
}

static int32_t sys_exec_impl(uint32_t path_addr, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)b; (void)c; (void)d; (void)e;
    if (!path_addr) return -1;

    const char *path = (const char*)path_addr;
    uint32_t size = 0;
    uint8_t *data = initrd_get_file(path, &size);

    if (!data) {
        printk("Syscall exec: Executable '%s' not found in initrd\n", path);
        return -1;
    }

    if (!current_process) return -1;

    if (elf_load_segments(data, size, current_process->page_directory) == 0) {
        current_process->entry_point = elf_get_entry(data);
        current_process->user_eip = current_process->entry_point;
        current_process->user_esp = PROC_STACK_BASE - 16;
        strncpy(current_process->name, path, PROC_NAME_LEN - 1);
        printk("Syscall exec: Replaced process image with '%s'\n", path);
        return 0;
    }
    return -1;
}

static int32_t sys_sbrk_impl(uint32_t increment, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)b; (void)c; (void)d; (void)e;
    if (!current_process) return -1;

    uint32_t old_brk = current_process->brk;
    if (old_brk + increment <= current_process->brk_max) {
        current_process->brk += increment;
        return (int32_t)old_brk;
    }

    // Allocate additional pages for user heap
    uint32_t new_pages = (increment + 4095) / 4096;
    for (uint32_t i = 0; i < new_pages; i++) {
        void *paddr = pmm_alloc_page();
        if (paddr) {
            vmm_map_page(current_process->brk_max + (i * 4096), (uint32_t)paddr, VMM_PRESENT | VMM_WRITABLE | VMM_USER);
        }
    }
    current_process->brk_max += (new_pages * 4096);
    current_process->brk += increment;
    return (int32_t)old_brk;
}

static int32_t sys_getpid_impl(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)a; (void)b; (void)c; (void)d; (void)e;
    return current_process ? (int32_t)current_process->pid : 0;
}

static int32_t sys_getppid_impl(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)a; (void)b; (void)c; (void)d; (void)e;
    if (current_process && current_process->parent) {
        return (int32_t)current_process->parent->pid;
    }
    return 0;
}

static int32_t sys_kill_impl(uint32_t pid, uint32_t sig, uint32_t c, uint32_t d, uint32_t e) {
    (void)c; (void)d; (void)e;
    process_t *target = process_get_by_pid(pid);
    if (!target) return -1;

    if (sig == 9) { // SIGKILL
        target->state = PROC_ZOMBIE;
        target->exit_code = 137;
        printk("Syscall kill: Process PID %u killed by signal 9\n", pid);
        return 0;
    }
    return -1;
}

static int32_t sys_mmap_impl(uint32_t addr, uint32_t len, uint32_t prot, uint32_t flags, uint32_t e) {
    (void)prot; (void)flags; (void)e;
    if (len == 0) return 0;

    uint32_t pages = (len + 4095) / 4096;
    uint32_t start_vaddr = addr ? addr : 0x40000000;

    for (uint32_t i = 0; i < pages; i++) {
        void *paddr = pmm_alloc_page();
        if (paddr) {
            vmm_map_page(start_vaddr + (i * 4096), (uint32_t)paddr, VMM_PRESENT | VMM_WRITABLE | VMM_USER);
        }
    }
    return (int32_t)start_vaddr;
}

static int32_t sys_munmap_impl(uint32_t addr, uint32_t len, uint32_t c, uint32_t d, uint32_t e) {
    (void)c; (void)d; (void)e;
    uint32_t pages = (len + 4095) / 4096;
    for (uint32_t i = 0; i < pages; i++) {
        vmm_unmap_page(addr + (i * 4096));
    }
    return 0;
}

static int32_t sys_gettimeofday_impl(uint32_t tv_addr, uint32_t tz, uint32_t c, uint32_t d, uint32_t e) {
    (void)tz; (void)c; (void)d; (void)e;
    if (!tv_addr) return -1;
    uint32_t *tv = (uint32_t*)tv_addr;
    tv[0] = rtc_get_unix_time();
    tv[1] = 0;
    return 0;
}

static int32_t sys_uname_impl(uint32_t buf_addr, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)b; (void)c; (void)d; (void)e;
    if (!buf_addr) return -1;
    char *buf = (char*)buf_addr;
    strncpy(buf, "Dionnex", 64);
    strncpy(buf + 65, "0.1", 64);
    strncpy(buf + 130, "i686", 64);
    return 0;
}

static int32_t sys_nanosleep_impl(uint32_t ms, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)b; (void)c; (void)d; (void)e;
    timer_sleep(ms);
    return 0;
}

static int32_t sys_init_module_impl(uint32_t buf_addr, uint32_t size, uint32_t name_addr, uint32_t d, uint32_t e) {
    (void)d; (void)e;
    if (!buf_addr || size == 0 || !name_addr) return -1;
    return module_load_binary((const char*)name_addr, (uint8_t*)buf_addr, size);
}

static int32_t sys_delete_module_impl(uint32_t name_addr, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)b; (void)c; (void)d; (void)e;
    if (!name_addr) return -1;
    return module_unload_binary((const char*)name_addr);
}

static int32_t sys_query_module_impl(uint32_t name_addr, uint32_t buf_addr, uint32_t c, uint32_t d, uint32_t e) {
    (void)name_addr; (void)buf_addr; (void)c; (void)d; (void)e;
    return 0;
}

void syscall_handler(registers_t *regs) {
    if (!regs) return;

    uint32_t num = regs->eax;
    if (num < DIONNEX_MAX_SYSCALLS && syscall_table[num]) {
        regs->eax = (uint32_t)syscall_table[num](regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
    } else {
        printk("Syscall: Invalid syscall number %u\n", num);
        regs->eax = (uint32_t)-1;
    }
}

void syscall_init(void) {
    for (int i = 0; i < DIONNEX_MAX_SYSCALLS; i++) {
        syscall_table[i] = NULL;
    }

    syscall_table[SYS_EXIT]         = sys_exit_impl;
    syscall_table[SYS_FORK]         = sys_fork_impl;
    syscall_table[SYS_READ]         = sys_read_impl;
    syscall_table[SYS_WRITE]        = sys_write_impl;
    syscall_table[SYS_OPEN]         = sys_open_impl;
    syscall_table[SYS_CLOSE]        = sys_close_impl;
    syscall_table[SYS_WAITPID]      = sys_waitpid_impl;
    syscall_table[SYS_EXEC]         = sys_exec_impl;
    syscall_table[SYS_SBRK]         = sys_sbrk_impl;
    syscall_table[SYS_GETPID]       = sys_getpid_impl;
    syscall_table[SYS_GETPPID]      = sys_getppid_impl;
    syscall_table[SYS_KILL]         = sys_kill_impl;
    syscall_table[SYS_MMAP]         = sys_mmap_impl;
    syscall_table[SYS_MUNMAP]       = sys_munmap_impl;
    syscall_table[SYS_GETTIMEOFDAY] = sys_gettimeofday_impl;
    syscall_table[SYS_UNAME]        = sys_uname_impl;
    syscall_table[SYS_NANOSLEEP]    = sys_nanosleep_impl;
    syscall_table[SYS_INIT_MODULE]   = sys_init_module_impl;
    syscall_table[SYS_DELETE_MODULE] = sys_delete_module_impl;
    syscall_table[SYS_QUERY_MODULE]  = sys_query_module_impl;
    syscall_table[SYS_EXIT_GROUP]   = sys_exit_impl;

    idt_set_gate(0x80, (uint32_t)syscall_isr_stub, 0x08, 0xEE); // DPL=3 user accessible

    printk("Syscall: System call interface initialized (int 0x80)\n");
}
