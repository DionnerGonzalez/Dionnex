/* ============================================================================
 * Dionnex OS Kernel Core Header - include/kernel.h
 * ============================================================================
 */
#ifndef KERNEL_H
#define KERNEL_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;
typedef uint32_t           size_t;
typedef uint32_t           uintptr_t;

#define NULL ((void*)0)

/* I/O Port operations */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

/* VGA Screen Driver */
void vga_clear(void);
void vga_puts(const char* str);
void vga_putc(char c);
void vga_set_color(uint8_t fg, uint8_t bg);
void printk(const char* fmt, ...);

/* Kernel Subsystems Initializers */
void gdt_init(void);
void idt_init(void);
void pic_remap(void);
void pit_init(uint32_t frequency);
void pmm_init(uint32_t mem_size);
void vmm_init(void);
void kmalloc_init(void);
void scheduler_init(void);
void tss_init(void);
void syscall_init(void);
void keyboard_init(void);
void vfs_init(void);
void initramfs_init(void);
void kernel_shell_start(void);

#endif
