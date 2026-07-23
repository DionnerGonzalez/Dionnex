/* ============================================================================
 * Dionnex Monolithic Kernel - System Call Interface Implementation
 * ============================================================================
 */
#include <kernel/syscall.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <kernel/idt.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <mm/pmm.h>
#include <sched/task.h>
#include <sched/scheduler.h>
#include <fs/vfs.h>
#include <io.h>

extern void syscall_isr_stub(void);

typedef int32_t (*syscall_fn_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

static syscall_fn_t syscall_table[DIONNEX_MAX_SYSCALLS];

static int32_t sys_write_impl(uint32_t fd, uint32_t buf_addr, uint32_t len, uint32_t unused1, uint32_t unused2) {
    (void)unused1; (void)unused2;
    if (fd == 1 || fd == 2) { // stdout / stderr
        const char *s = (const char*)buf_addr;
        for (uint32_t i = 0; i < len; i++) {
            vga_putchar(s[i]);
        }
        return (int32_t)len;
    }
    return -1;
}

static int32_t sys_read_impl(uint32_t fd, uint32_t buf_addr, uint32_t len, uint32_t unused1, uint32_t unused2) {
    (void)unused1; (void)unused2;
    if (fd == 0) { // stdin
        char *s = (char*)buf_addr;
        uint32_t read_bytes = 0;
        while (read_bytes < len) {
            char c = keyboard_read();
            s[read_bytes++] = c;
            vga_putchar(c);
            if (c == '\n') break;
        }
        return (int32_t)read_bytes;
    }
    return -1;
}

static int32_t sys_exit_impl(uint32_t code, uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    printk("Task %u exited with code %d\n", current_task ? current_task->pid : 0, code);
    task_exit();
    return 0;
}

static int32_t sys_getpid_impl(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)a; (void)b; (void)c; (void)d; (void)e;
    return current_task ? (int32_t)current_task->pid : 0;
}

static int32_t sys_yield_impl(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)a; (void)b; (void)c; (void)d; (void)e;
    task_yield();
    return 0;
}

static int32_t sys_uptime_impl(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)a; (void)b; (void)c; (void)d; (void)e;
    return (int32_t)(timer_get_ticks() / 1000);
}

static int32_t sys_meminfo_impl(uint32_t buf_addr, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    (void)a; (void)b; (void)c; (void)d;
    if (!buf_addr) return -1;
    uint32_t *info = (uint32_t*)buf_addr;
    info[0] = pmm_get_total_count() * 4; // Total KB
    info[1] = pmm_get_free_count() * 4;  // Free KB
    info[2] = (info[0] - info[1]);       // Used KB
    return 0;
}

static int32_t sys_open_impl(uint32_t path_addr, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    (void)a; (void)b; (void)c; (void)d;
    vfs_node_t *node = vfs_open((const char*)path_addr);
    return node ? (int32_t)(uintptr_t)node : -1;
}

static int32_t sys_close_impl(uint32_t handle, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    (void)a; (void)b; (void)c; (void)d;
    vfs_node_t *node = (vfs_node_t*)(uintptr_t)handle;
    return vfs_close(node);
}

static int32_t sys_fread_impl(uint32_t handle, uint32_t buf_addr, uint32_t len, uint32_t a, uint32_t b) {
    (void)a; (void)b;
    vfs_node_t *node = (vfs_node_t*)(uintptr_t)handle;
    return vfs_read(node, 0, len, (uint8_t*)buf_addr);
}

static int32_t sys_fwrite_impl(uint32_t handle, uint32_t buf_addr, uint32_t len, uint32_t a, uint32_t b) {
    (void)a; (void)b;
    vfs_node_t *node = (vfs_node_t*)(uintptr_t)handle;
    return vfs_write(node, 0, len, (const uint8_t*)buf_addr);
}

static int32_t sys_sleep_impl(uint32_t ms, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    (void)a; (void)b; (void)c; (void)d;
    timer_sleep(ms);
    return 0;
}

static int32_t sys_reboot_impl(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    (void)a; (void)b; (void)c; (void)d; (void)e;
    outb(0x64, 0xFE);
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

    syscall_table[SYS_WRITE]   = sys_write_impl;
    syscall_table[SYS_READ]    = sys_read_impl;
    syscall_table[SYS_EXIT]    = sys_exit_impl;
    syscall_table[SYS_GETPID]  = sys_getpid_impl;
    syscall_table[SYS_YIELD]   = sys_yield_impl;
    syscall_table[SYS_UPTIME]  = sys_uptime_impl;
    syscall_table[SYS_MEMINFO] = sys_meminfo_impl;
    syscall_table[SYS_OPEN]    = sys_open_impl;
    syscall_table[SYS_CLOSE]   = sys_close_impl;
    syscall_table[SYS_FREAD]   = sys_fread_impl;
    syscall_table[SYS_FWRITE]  = sys_fwrite_impl;
    syscall_table[SYS_SLEEP]   = sys_sleep_impl;
    syscall_table[SYS_REBOOT]  = sys_reboot_impl;

    idt_set_gate(0x80, (uint32_t)syscall_isr_stub, 0x08, 0xEE); // DPL=3 user accessible

    printk("Syscall: System call interface initialized (int 0x80)\n");
}
