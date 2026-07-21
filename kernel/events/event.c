/*
 * Kernel Event Notification Engine
 */

#include <ABI/kernel/internal/kabi.h>

struct event_notifier {
	int (*notifier_call)(struct event_notifier *nb, unsigned long val, void *v);
	struct event_notifier *next;
};

void event_notify(struct event_notifier *list, unsigned long val, void *v)
{
	while (list) {
		list->notifier_call(list, val, v);
		list = list->next;
	}
}
