/* ============================================================================
 * Dionnex Monolithic Kernel - PS/2 Keyboard Driver Implementation
 * ============================================================================
 */
#include <drivers/keyboard.h>
#include <drivers/pic.h>
#include <kernel/idt.h>
#include <kernel/printk.h>
#include <io.h>

#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT 0x64

static uint8_t kb_buffer[256];
static uint8_t kb_head = 0;
static uint8_t kb_tail = 0;
static uint8_t shift_pressed = 0;
static uint8_t caps_lock = 0;

static const char scancode_set1[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
   '*',  0,  ' '
};

static const char scancode_set1_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
   '*',  0,  ' '
};

static void keyboard_handler(registers_t *regs) {
    (void)regs;
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }
    if (scancode & 0x80) {
        return; // Key release, ignore
    }

    char c = 0;
    if (scancode < 128) {
        c = shift_pressed ? scancode_set1_shift[scancode] : scancode_set1[scancode];
        if (caps_lock && c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        } else if (caps_lock && c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
    }

    if (c != 0) {
        uint8_t next_head = (kb_head + 1) % 256;
        if (next_head != kb_tail) {
            kb_buffer[kb_head] = c;
            kb_head = next_head;
        }
    }
}

void keyboard_init(void) {
    isr_register_handler(33, keyboard_handler);
    pic_enable_irq(1);
    printk("Keyboard: PS/2 Keyboard Driver initialized on IRQ1\n");
}

int keyboard_has_input(void) {
    return (kb_head != kb_tail);
}

char keyboard_read(void) {
    while (kb_head == kb_tail) {
        asm volatile("hlt");
    }
    char c = kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % 256;
    return c;
}
