/* ============================================================================
 * Dionnex Kernel - System Call Vector Dispatcher (syscall/syscall_table.c)
 * ============================================================================
 */
#include <kernel/kernel.h>
#include <kernel/kabi.h>

static int sys_write_impl(int fd, const char* buf, size_t count) {
    (void)fd;
    if (!buf) return -1;
    for (size_t i = 0; i < count; i++) {
        vga_putc(buf[i]);
    }
    return (int)count;
}

static int sys_getpid_impl(void) {
    return 1; // Return PID of running task
}

static int sys_ticks_impl(void) {
    return (int)pit_get_ticks();
}

int syscall_dispatch(int num, int arg1, int arg2, int arg3) {
    switch (num) {
        case SYS_WRITE:
            return sys_write_impl(arg1, (const char*)arg2, (size_t)arg3);
        case SYS_GETPID:
            return sys_getpid_impl();
        case SYS_TICKS:
            return sys_ticks_impl();
        case SYS_MALLOC:
            return (int)kmalloc((size_t)arg1);
        case SYS_EXIT:
            kprintf("\n[SYSCALL] Process exited with status code: %d\n", arg1);
            return 0;
        default:
            kprintf("\n[SYSCALL] Invalid system call invoked: %d\n", num);
            return -1;
    }
}

void syscall_init(void) {
    // INT 0x80 System Call Entry is set up in IDT
}
