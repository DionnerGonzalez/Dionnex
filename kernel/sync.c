/* ============================================================================
 * Dionnex Monolithic Kernel - Synchronization Primitives Implementation
 * ============================================================================
 */
#include <kernel/sync.h>
#include <kernel/smp.h>
#include <sched/task.h>
#include <io.h>

static inline uint32_t atomic_xchg32(volatile uint32_t *ptr, uint32_t new_val) {
    uint32_t old_val;
    asm volatile("xchgl %0, %1"
                 : "=r"(old_val), "+m"(*ptr)
                 : "0"(new_val)
                 : "memory");
    return old_val;
}

void spinlock_init(spinlock_t *lock) {
    if (!lock) return;
    lock->locked = 0;
    lock->owner_cpu = 0;
}

void spin_lock(spinlock_t *lock) {
    if (!lock) return;
    while (atomic_xchg32(&lock->locked, 1U) != 0U) {
        while (lock->locked) {
            cpu_pause();
        }
    }
    lock->owner_cpu = smp_get_current_cpu();
}

void spin_unlock(spinlock_t *lock) {
    if (!lock) return;
    asm volatile("" ::: "memory");
    lock->locked = 0;
}

uint32_t spin_lock_irqsave(spinlock_t *lock) {
    uint32_t eflags = read_eflags();
    cpu_cli();
    spin_lock(lock);
    return eflags;
}

void spin_unlock_irqrestore(spinlock_t *lock, uint32_t flags) {
    spin_unlock(lock);
    if (flags & (1U << 9)) {
        cpu_sti();
    }
}

void mutex_init(mutex_t *m) {
    if (!m) return;
    spinlock_init(&m->guard);
    m->locked = 0;
    m->owner_pid = 0;
}

int mutex_trylock(mutex_t *m) {
    if (!m) return 0;
    uint32_t flags = spin_lock_irqsave(&m->guard);
    int acquired = 0;
    if (!m->locked) {
        m->locked = 1;
        task_t *t = task_get_current();
        m->owner_pid = t ? t->pid : 0;
        acquired = 1;
    }
    spin_unlock_irqrestore(&m->guard, flags);
    return acquired;
}

void mutex_lock(mutex_t *m) {
    if (!m) return;
    while (!mutex_trylock(m)) {
        if (read_eflags() & (1U << 9)) {
            task_yield();
        } else {
            cpu_pause();
        }
    }
}

void mutex_unlock(mutex_t *m) {
    if (!m) return;
    uint32_t flags = spin_lock_irqsave(&m->guard);
    m->locked = 0;
    m->owner_pid = 0;
    spin_unlock_irqrestore(&m->guard, flags);
}

void sem_init(semaphore_t *s, int32_t initial_count) {
    if (!s) return;
    spinlock_init(&s->guard);
    s->count = initial_count;
}

int sem_trywait(semaphore_t *s) {
    if (!s) return 0;
    uint32_t flags = spin_lock_irqsave(&s->guard);
    int acquired = 0;
    if (s->count > 0) {
        s->count--;
        acquired = 1;
    }
    spin_unlock_irqrestore(&s->guard, flags);
    return acquired;
}

void sem_wait(semaphore_t *s) {
    if (!s) return;
    while (!sem_trywait(s)) {
        if (read_eflags() & (1U << 9)) {
            task_yield();
        } else {
            cpu_pause();
        }
    }
}

void sem_post(semaphore_t *s) {
    if (!s) return;
    uint32_t flags = spin_lock_irqsave(&s->guard);
    s->count++;
    spin_unlock_irqrestore(&s->guard, flags);
}
