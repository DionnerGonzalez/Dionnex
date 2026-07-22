/* ============================================================================
 * Dionnex Kernel - PS/2 Keyboard Driver (drivers/keyboard.c)
 * ============================================================================
 */
#include <kernel.h>

#define KEYBOARD_PORT 0x60

static const char scancode_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static char kb_buffer[256];
static uint32_t kb_head = 0;
static uint32_t kb_tail = 0;

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_PORT);
    if (!(scancode & 0x80)) { // Key press
        char ascii = scancode_ascii[scancode];
        if (ascii != 0) {
            kb_buffer[kb_head % 256] = ascii;
            kb_head++;
        }
    }
    outb(0x20, 0x20); // EOI
}

void keyboard_init(void) {
    vga_puts("[KEYBOARD] Controlador PS/2 Teclado iniciado (Puerto 0x60, IRQ1).\n");
}

char keyboard_read_char(void) {
    if (kb_tail == kb_head) return 0;
    char c = kb_buffer[kb_tail % 256];
    kb_tail++;
    return c;
}
