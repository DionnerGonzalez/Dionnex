/* ============================================================================
 * Dionnex Monolithic Kernel - PS/2 Keyboard Driver Implementation
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/keyboard.h>
#include <drivers/pic.h>
#include <kernel/idt.h>
#include <kernel/printk.h>
#include <io.h>

#define KEYBOARD_DATA_PORT   0x60

static uint8_t kb_buffer[256];
static volatile uint8_t kb_head = 0;
static volatile uint8_t kb_tail = 0;
static uint8_t shift_pressed = 0;
static uint8_t ctrl_pressed  = 0;
static uint8_t alt_pressed   = 0;
static uint8_t caps_lock     = 0;
static uint8_t ext_prefix    = 0;

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

static void kb_push(uint8_t byte) {
    uint8_t next_head = (uint8_t)((kb_head + 1U) % 256U);
    if (next_head != kb_tail) {
        kb_buffer[kb_head] = byte;
        kb_head = next_head;
    }
}

static void keyboard_handler(registers_t *regs) {
    (void)regs;
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == 0xE0U) {
        ext_prefix = 1;
        return;
    }

    if (ext_prefix) {
        ext_prefix = 0;
        if (scancode == 0x1DU) { ctrl_pressed = 1; return; }
        if (scancode == 0x9DU) { ctrl_pressed = 0; return; }
        if (scancode == 0x38U) { alt_pressed = 1; return; }
        if (scancode == 0xB8U) { alt_pressed = 0; return; }
        if (scancode & 0x80U) return;

        switch (scancode) {
            case 0x48U: kb_push(KEY_SPECIAL_UP);    return;
            case 0x50U: kb_push(KEY_SPECIAL_DOWN);  return;
            case 0x4BU: kb_push(KEY_SPECIAL_LEFT);  return;
            case 0x4DU: kb_push(KEY_SPECIAL_RIGHT); return;
            default: return;
        }
    }

    if (scancode == 0x2AU || scancode == 0x36U) { shift_pressed = 1; return; }
    if (scancode == 0xAAU || scancode == 0xB6U) { shift_pressed = 0; return; }
    if (scancode == 0x1DU) { ctrl_pressed = 1; return; }
    if (scancode == 0x9DU) { ctrl_pressed = 0; return; }
    if (scancode == 0x38U) { alt_pressed = 1; return; }
    if (scancode == 0xB8U) { alt_pressed = 0; return; }
    if (scancode == 0x3AU) { caps_lock = !caps_lock; return; }

    if (scancode & 0x80U) return;

    char c = 0;
    if (scancode < 128U) {
        c = shift_pressed ? scancode_set1_shift[scancode] : scancode_set1[scancode];
        if (caps_lock && c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        } else if (caps_lock && c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
        if (ctrl_pressed && c >= 'a' && c <= 'z') {
            c = (char)(c - 'a' + 1);
        }
    }

    if (c != 0) {
        kb_push((uint8_t)c);
    }
}

void keyboard_init(void) {
    isr_register_handler(33, keyboard_handler);
    pic_enable_irq(1);
    printk("Keyboard: PS/2 Keyboard Driver initialized on IRQ1 (0xE0/Ctrl/Alt)\n");
}

int keyboard_has_input(void) {
    return (kb_head != kb_tail);
}

int keyboard_is_ctrl_pressed(void) { return ctrl_pressed; }
int keyboard_is_alt_pressed(void)  { return alt_pressed; }

char keyboard_read(void) {
    while (kb_head == kb_tail) {
        asm volatile("hlt");
    }
    char c = (char)kb_buffer[kb_tail];
    kb_tail = (uint8_t)((kb_tail + 1U) % 256U);
    return c;
}
