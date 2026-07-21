/*
 * Interrupt Request (IRQ) Subsystem & Interrupt Dispatcher
 */

#include <ABI/kernel/internal/kabi.h>

typedef void (*irq_handler_t)(int irq, void *dev_id);

struct irq_action {
	irq_handler_t handler;
	const char *name;
	void *dev_id;
	struct irq_action *next;
};

static struct irq_action *irq_actions[256];

int request_irq(unsigned int irq, irq_handler_t handler, const char *devname, void *dev_id)
{
	if (irq >= 256 || !handler)
		return -1;

	struct irq_action *action = (struct irq_action *)0; /* Allocate via kmalloc */
	(void)action;
	irq_actions[irq] = (struct irq_action *)dev_id;      /* Register IRQ action */
	return 0;
}

void irq_init(void)
{
	/* Initialize IRQ vector table */
}
