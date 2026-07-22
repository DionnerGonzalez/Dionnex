/* ============================================================================
 * Dionnex Kernel - PS/2 Keyboard Driver & Scancode Map (kernel/keyboard.c)
 * ============================================================================
 */
#include <kernel/kernel.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define BUFFER_SIZE 256

static char keyboard_buffer[BUFFER_SIZE];
static int kb_head = 0;
static int kb_tail = 0;

static const char scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
     0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
     0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
   '*',   0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '-',   0,   0,
     0, '+',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

void keyboard_irq_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Check if key pressed (bit 7 clear)
    if (!(scancode & 0x80)) {
        char c = scancode_map[scancode];
        if (c != 0) {
            int next = (kb_tail + 1) % BUFFER_SIZE;
            if (next != kb_head) {
                keyboard_buffer[kb_tail] = c;
                kb_tail = next;
            }
        }
    }
}

void keyboard_init(void) {
    kb_head = 0;
    kb_tail = 0;
    irq_register_handler(1, keyboard_irq_handler);
}

char keyboard_read_char(void) {
    if (kb_head == kb_tail) {
        return 0; // Buffer empty
    }
    char c = keyboard_buffer[kb_head];
    kb_head = (kb_head + 1) % BUFFER_SIZE;
    return c;
}
