/*
 * Atomic Spinlock Synchronization Primitives
 */

#include <ABI/kernel/internal/kabi.h>

typedef struct {
	volatile u32 counter;
} spinlock_t;

void spin_lock_init(spinlock_t *lock)
{
	if (lock)
		lock->counter = 0;
}

void spin_lock(spinlock_t *lock)
{
	if (!lock) return;
	while (__sync_lock_test_and_set(&lock->counter, 1)) {
		__asm__ __volatile__("pause");
	}
}

void spin_unlock(spinlock_t *lock)
{
	if (!lock) return;
	__sync_lock_release(&lock->counter);
}
