# DionOS Kernel Core API Documentation

## Logging Subsystem
- `pr_info(fmt, ...)`: Print informational kernel logs.
- `pr_err(fmt, ...)`: Print error logs.
- `pr_warn(fmt, ...)`: Print warning logs.
- `pr_debug(fmt, ...)`: Debug logs (active when CONFIG_DEBUG is enabled).

## Memory Allocation API
- `kmalloc(size_t size, gfp_t flags)`: Allocate contiguous kernel heap bytes.
- `kfree(const void *ptr)`: Release memory back to the slab/page allocator.
- `get_free_page(gfp_t flags)`: Allocate a single 4KB physical frame.
- `free_page(unsigned long addr)`: Release frame back to Buddy physical frame allocator.

## Scheduler API
- `schedule(void)`: Yield current execution context and invoke CFS scheduler.
- `wake_up_process(struct task_struct *p)`: Move process from sleeping to ready runqueue.
- `kthread_create(int (*threadfn)(void *data), void *data, const char namefmt[])`: Spawn kernel thread.

## Synchronization API
- `spin_lock_init(spinlock_t *lock)`: Initialize spinlock.
- `spin_lock(spinlock_t *lock)`: Acquire non-blocking lock.
- `spin_unlock(spinlock_t *lock)`: Release spinlock.
