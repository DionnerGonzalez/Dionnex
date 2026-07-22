/* ============================================================================
 * Dionnex OS - Kernel Main Entry Point (kernel/main.c)
 * ============================================================================
 */
#include <kernel.h>

/* VGA Text Mode Buffer */
static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static uint8_t vga_color = 0x0F; // Light white on black
static size_t vga_row = 0;
static size_t vga_col = 0;

void vga_clear(void) {
    for (size_t y = 0; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            const size_t index = y * 80 + x;
            vga_buffer[index] = (uint16_t)' ' | ((uint16_t)vga_color << 8);
        }
    }
    vga_row = 0;
    vga_col = 0;
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else {
        const size_t index = vga_row * 80 + vga_col;
        vga_buffer[index] = (uint16_t)c | ((uint16_t)vga_color << 8);
        vga_col++;
        if (vga_col >= 80) {
            vga_col = 0;
            vga_row++;
        }
    }
    if (vga_row >= 25) {
        vga_clear();
    }
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = (bg << 4) | (fg & 0x0F);
}

void printk(const char* str, ...) {
    vga_puts(str);
}

void kernel_main(uint32_t magic, void* multiboot_addr) {
    (void)magic;
    (void)multiboot_addr;

    vga_clear();
    vga_set_color(10, 0); // Light Green on Black
    vga_puts("===================================================================\n");
    vga_puts("      Dionnex OS Kernel v1.0.0 (x86 Protected Mode 32-bit)          \n");
    vga_puts("===================================================================\n\n");

    vga_set_color(14, 0); // Yellow
    vga_puts("Hola! Kernel iniciado con exito desde boot.asm + linker.ld!\n\n");
    vga_set_color(15, 0); // White

    /* 1. Subsystems Initialization Sequence */
    gdt_init();        // 2. GDT Setup
    idt_init();        // 2. IDT Setup
    pic_remap();       // 2. PIC Remap (0x20, 0x28)
    pit_init(100);     // 3. PIT Timer (100Hz ticks)
    pmm_init(128);     // 4. PMM Bitmap Allocator (128 MB)
    vmm_init();        // 5. VMM Page Directory & Tables
    kmalloc_init();    // 6. Heap Dynamic Allocator
    tss_init();        // 9. Task State Segment
    syscall_init();    // 9. Syscall int 0x80 interface
    scheduler_init();  // 7 & 8. Round-Robin Scheduler & Context Switch
    keyboard_init();   // 11. PS/2 Keyboard Driver
    vfs_init();        // 11. Virtual File System
    initramfs_init();  // 11. InitRAMFS Ramdisk

    vga_set_color(11, 0); // Cyan
    vga_puts("\n[KERNEL] Todos los 11 subsistemas nucleares inicializados.\n");
    vga_puts("[KERNEL] Iniciando Dionnex Kernel Shell (dionnex-sh)...\n\n");
    vga_set_color(15, 0);

    /* Enable Interrupts for PIT timer & Keyboard */
    __asm__ __volatile__("sti");

    kernel_shell_start();
}
