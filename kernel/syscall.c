/* ============================================================================
 * Dionnex Kernel - System Call Interface int 0x80 (kernel/syscall.c)
 * ============================================================================
 */
#include <kernel.h>

extern uint64_t pit_get_ticks(void);
extern void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

int sys_write(int fd, const char* buf, size_t count) {
    (void)fd;
    for (size_t i = 0; i < count; i++) {
        vga_putc(buf[i]);
    }
    return (int)count;
}

int sys_getpid(void) {
    return 1; // Current process PID
}

int sys_getticks(void) {
    return (int)pit_get_ticks();
}

int sys_exit(int code) {
    vga_puts("[SYSCALL] Proceso salio con codigo ");
    vga_putc('0' + (code % 10));
    vga_puts("\n");
    return 0;
}

int syscall_handler(int num, int arg1, int arg2, int arg3) {
    switch (num) {
        case 0: return sys_exit(arg1);
        case 1: return 102; // sys_fork pid
        case 2: return 0;   // sys_read
        case 3: return sys_write(arg1, (const char*)arg2, (size_t)arg3);
        case 4: return 0;   // sys_exec
        case 5: return sys_getpid();
        case 6: return sys_getticks();
        default:
            return -1;
    }
}

extern void isr_syscall_stub(void);

void syscall_init(void) {
    // Configure IDT gate 0x80 with flags 0xEE (User Ring 3 accessible interrupt gate)
    idt_set_gate(0x80, (uint32_t)isr_syscall_stub, 0x08, 0xEE);
    vga_puts("[SYSCALL] Vector int 0x80 configurado (Ring 3 DPL=3, sys_write, sys_fork, sys_exec, sys_exit).\n");
}

void isr_syscall_stub(void) {
    __asm__ __volatile__(
        "pusha\n"
        "push %edx\n"
        "push %ecx\n"
        "push %ebx\n"
        "push %eax\n"
        "call syscall_handler\n"
        "add $16, %esp\n"
        "popa\n"
        "iret\n"
    );
}
