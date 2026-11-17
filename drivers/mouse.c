/* ============================================================================
 * Dionnex Monolithic Kernel - PS/2 Mouse Driver Implementation (IRQ12)
 * License: GNU GPL v2 (GPL-2.0-only)
 * Author: Ihosvanni Dionner Gonzalez
 * ============================================================================
 */
#include <drivers/mouse.h>
#include <drivers/pic.h>
#include <kernel/idt.h>
#include <kernel/printk.h>
#include <io.h>

#define PS2_DATA_PORT   0x60U
#define PS2_STATUS_PORT 0x64U

static mouse_state_t g_mouse = { 400, 300, 0, 0 };
static uint8_t mouse_cycle = 0;
static int8_t  mouse_bytes[3];

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000U;
    if (type == 0) {
        while (timeout--) {
            if (inb(PS2_STATUS_PORT) & 1U) return;
        }
    } else {
        while (timeout--) {
            if (!(inb(PS2_STATUS_PORT) & 2U)) return;
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(PS2_STATUS_PORT, 0xD4U);
    mouse_wait(1);
    outb(PS2_DATA_PORT, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(PS2_DATA_PORT);
}

static void mouse_irq_handler(registers_t *regs) {
    (void)regs;
    uint8_t status = inb(PS2_STATUS_PORT);
    if (!(status & 0x20U)) return;

    int8_t byte = (int8_t)inb(PS2_DATA_PORT);

    if (mouse_cycle == 0 && !((uint8_t)byte & 0x08U)) {
        return; /* Wait for alignment bit 3 */
    }

    mouse_bytes[mouse_cycle++] = byte;
    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        uint8_t flags = (uint8_t)mouse_bytes[0];
        if (flags & 0xC0U) return; /* Overflow */

        int32_t dx = mouse_bytes[1];
        int32_t dy = mouse_bytes[2];

        g_mouse.buttons = flags & 0x07U;
        g_mouse.x += dx;
        g_mouse.y -= dy;
        if (g_mouse.x < 0) g_mouse.x = 0;
        if (g_mouse.y < 0) g_mouse.y = 0;
        if (g_mouse.x > 1920) g_mouse.x = 1920;
        if (g_mouse.y > 1080) g_mouse.y = 1080;
        g_mouse.packets++;
    }
}

void mouse_init(void) {
    mouse_wait(1);
    outb(PS2_STATUS_PORT, 0xA8U); /* Enable auxiliary PS/2 device */

    mouse_wait(1);
    outb(PS2_STATUS_PORT, 0x20U); /* Get controller config byte */
    uint8_t status = (mouse_read() | 0x02U) & ~0x20U;
    mouse_wait(1);
    outb(PS2_STATUS_PORT, 0x60U);
    mouse_wait(1);
    outb(PS2_DATA_PORT, status);

    mouse_write(0xF6U); /* Set defaults */
    mouse_read();
    mouse_write(0xF4U); /* Enable packet streaming */
    mouse_read();

    isr_register_handler(44, mouse_irq_handler);
    pic_enable_irq(2);
    pic_enable_irq(12);
    printk("Mouse: PS/2 Mouse Driver initialized on IRQ12\n");
}

void mouse_get_state(mouse_state_t *out) {
    if (!out) return;
    *out = g_mouse;
}
