/* ============================================================================
 * Dionnex Monolithic Kernel - Synchronization Primitives Header
 * ============================================================================
 */
#ifndef _DIONNEX_SYNC_H_
#define _DIONNEX_SYNC_H_

#include <types.h>

typedef struct spinlock {
    volatile uint32_t locked;
    uint32_t owner_cpu;
} spinlock_t;

typedef struct mutex {
    spinlock_t guard;
    volatile uint32_t locked;
    uint32_t owner_pid;
} mutex_t;

typedef struct semaphore {
    spinlock_t guard;
    volatile int32_t count;
} semaphore_t;

#define SPINLOCK_INIT { 0, 0 }
#define MUTEX_INIT    { SPINLOCK_INIT, 0, 0 }

void spinlock_init(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
uint32_t spin_lock_irqsave(spinlock_t *lock);
void spin_unlock_irqrestore(spinlock_t *lock, uint32_t flags);

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
int mutex_trylock(mutex_t *m);
void mutex_unlock(mutex_t *m);

void sem_init(semaphore_t *s, int32_t initial_count);
void sem_wait(semaphore_t *s);
int sem_trywait(semaphore_t *s);
void sem_post(semaphore_t *s);

#endif /* _DIONNEX_SYNC_H_ */
