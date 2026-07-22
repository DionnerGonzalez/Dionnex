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

/* Utility Memory & String Functions */
void* memset(void* dest, int val, size_t count);
void* memcpy(void* dest, const void* src, size_t count);
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcpy(char* dest, const char* src);

/* Interrupt Management */
static inline void cli(void) {
    __asm__ __volatile__ ("cli");
}

static inline void sti(void) {
    __asm__ __volatile__ ("sti");
}

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

/* Memory Management */
void pmm_init(uint32_t mem_size);
void* pmm_alloc_page(void);
void pmm_free_page(void* paddr);
uint32_t pmm_get_used_pages(void);
uint32_t pmm_get_total_pages(void);

void vmm_init(void);
int vmm_map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags);
void vmm_unmap_page(uint32_t vaddr);

void kmalloc_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

/* Kernel Subsystems Initializers */
void gdt_init(void);
void idt_init(void);
void pic_remap(void);
void pit_init(uint32_t frequency);
uint64_t pit_get_ticks(void);
void scheduler_init(void);
void schedule(void);
void tss_init(void);
void tss_set_kernel_stack(uint32_t kstack);
void syscall_init(void);
void keyboard_init(void);
char keyboard_read_char(void);
void vfs_init(void);
int vfs_add_file(const char* name, const char* content, uint32_t size);
void vfs_ls(void);
const char* vfs_read_file(const char* name);
void initramfs_init(void);
void kernel_shell_start(void);
void process_shell_command(const char* cmd);

#endif

